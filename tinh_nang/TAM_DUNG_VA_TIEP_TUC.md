# Tạm dừng và tiếp tục — Pause/Resume

## 1. Ví dụ dễ hình dung

Giả sử rắn NORMAL đang có 6 đốt, 80 điểm, 2 thanh chắn và đã tích lũy 7 giây RUNNING kể từ lượt thử sinh vật cản gần nhất. Người chơi nhấn 5 để Pause.

| Dữ liệu | Khi đang chạy | Trong lúc Pause | Sau Resume |
|---|---|---|---|
| Trạng thái | RUNNING | PAUSED | RUNNING |
| Vị trí, chiều dài và điểm | Đang thay đổi theo ván | Giữ nguyên | Chơi tiếp từ dữ liệu đó |
| Mồi, tường và thanh chắn | Dữ liệu hiện tại | Giữ nguyên | Tiếp tục dùng |
| Thời gian sinh vật cản đã tích lũy | Khoảng 7 giây | Giữ phần tích lũy | Cần thêm khoảng 3 giây RUNNING |
| Quét phím và LED | Hoạt động | Vẫn hoạt động | Vẫn hoạt động |

Nhấn rồi nhả phím 5 một lần để dừng, nhấn rồi nhả lần nữa để tiếp tục. Giữ phím không tự lặp Pause/Resume vì bộ quét khóa đến khi nhả ổn định.

```text
RUNNING ──5──> PAUSED ──5──> RUNNING
                    │
                    └──9──> Tạo lại ván, điểm 0, rắn 3 đốt
```

Đây là tạm dừng ván bên trong firmware. Đồng hồ và vòng lặp MCU vẫn chạy để tiếp tục nhận nút.

## 2. Cách chương trình thực hiện

Nhánh đổi trạng thái nằm trong [game/game.c](../../game/game.c); các mốc thời gian và yêu cầu hiển thị được xử lý trong [main.c](../../main.c).

### 2.1. Một trạng thái riêng cho Pause

```c
typedef enum {
    SNAKE_RUNNING=0,
    SNAKE_PAUSED,
    SNAKE_GAME_OVER,
    SNAKE_WIN
} SnakeState;
```

`SNAKE_PAUSED` khác GAME OVER và WIN. Hai trạng thái kết thúc không được phím 5 hồi sinh thành RUNNING; chỉ PAUSED được tiếp tục bằng Resume.

Không có một biến “bản sao ván trước Pause”. Mảng thân, mồi, điểm, chiều dài, hướng và vật cản vẫn nằm trong các biến hiện tại. Chương trình giữ dữ liệu bằng cách ngừng các hàm làm tiến triển ván.

### 2.2. Phím 5 đổi trạng thái theo hai chiều

```c
if (key==KEY_PAUSE) {
    if (state==SNAKE_RUNNING) state=SNAKE_PAUSED;
    else if (state==SNAKE_PAUSED) state=SNAKE_RUNNING;
    return;
}
```

Nhánh này nằm trước kiểm tra chặn nhận hướng khi không RUNNING. Vì vậy, khi đã PAUSED, sự kiện phím 5 vẫn tới được nhánh Resume.

- RUNNING → PAUSED khi nhấn 5.
- PAUSED → RUNNING khi nhấn lại 5.
- GAME OVER/WIN → giữ nguyên nếu nhấn 5.

Lệnh `return` kết thúc xử lý sự kiện đó; phím 5 không bị hiểu tiếp như một lệnh rẽ. Cơ chế chống dội và nhấn/thả được trình bày trong [DIEU_KHIEN_KEYPAD.md](DIEU_KHIEN_KEYPAD.md).

### 2.3. `main.c` phát hiện trạng thái vừa thay đổi

Trước khi xử lý phím, `main.c` lưu `before=snake_state()`, gọi `snake_handle_key(key)` rồi so sánh lại trạng thái:

```c
if (key==KEY_RESTART || before!=snake_state()) {
    redraw=1;
    update_lcd=1;
    if (key==KEY_RESTART || snake_state()==SNAKE_RUNNING)
        last_move_ms=now_ms;
}
```

Nếu Pause hoặc Resume thành công, `before!=snake_state()` nên bật cả `redraw` và `update_lcd`. Điều này khiến dòng trạng thái thay đổi ngay mà không phải đợi đầu rắn đi thêm một ô.

Nếu trạng thái mới là RUNNING, chương trình đặt `last_move_ms=now_ms`. Nếu vừa Pause, nó chưa đặt lại mốc này; mốc sẽ được đặt lại khi Resume.

### 2.4. Chặn bước di chuyển tại hai lớp

Điều kiện gọi `snake_step()` trong `main.c` yêu cầu rắn RUNNING. Bên trong hàm cũng có:

```c
if (state!=SNAKE_RUNNING) return 0;
```

Vì vậy, trong Pause không dịch `segments[]`, không tạo đầu mới, không kiểm tra ăn để tăng điểm/chiều dài và không đảo đầu–đuôi. Mồi không đổi vì không có bước ăn gọi `place_food()`.

Đây là dừng cập nhật gameplay; code không dùng vòng `while` riêng để đứng chờ phím 5, nên các thiết bị vẫn được phục vụ.

### 2.5. Không nhận lệnh rẽ mới khi đang Pause

Sau nhánh Pause, phần nhận hướng có điều kiện:

```c
if (state!=SNAKE_RUNNING || turn_queued) return;
```

Các phím 2/4/6/8 khi PAUSED không đổi hướng. Tuy nhiên, nếu một lệnh rẽ đã được nhận **trước** khi Pause và chưa đến bước đi, nhánh Pause không xóa `turn_queued` hoặc `next_direction`.

Ví dụ đang đi RIGHT, đã nhận UP rồi người chơi Pause trước bước di chuyển: yêu cầu UP vẫn tồn tại. Sau Resume, đến bước đi kế tiếp rắn có thể dùng lệnh UP đó. Đây là tiếp tục dữ liệu đang có, không khởi tạo lại hướng.

### 2.6. Loại thời gian Pause khỏi thời gian sinh vật cản

Đầu vòng lặp chính:

```c
running_elapsed_ms=0;
if (app_state==APP_STATE_GAME && snake_state()==SNAKE_RUNNING) {
    running_elapsed_ms=(unsigned long)(now_ms-last_service_ms);
}
last_service_ms=now_ms;
```

`last_service_ms` được cập nhật ở mọi lượt, dù đang chạy, Pause hay ở menu. Chỉ RUNNING trong màn game mới nhận khoảng chênh lệch vào `running_elapsed_ms`.

Nếu không cập nhật mốc khi Pause, lần Resume có thể lấy `now_ms` trừ một mốc quá cũ rồi tính nhầm toàn bộ thời gian dừng. Cách hiện tại tránh việc đó.

`dynamic_obstacle_update()` chỉ được gọi khi trạng thái sau xử lý phím vẫn RUNNING. Trong Pause, biến `elapsed_ms` bên bộ vật cản không được cộng thêm và các thanh không thay đổi.

### 2.7. Ví dụ đồng hồ theo từng thời điểm

Giả sử đã tích lũy 7.000 ms cho vật cản:

| Thời điểm minh họa | Trạng thái và hành động | Tích lũy vật cản |
|---|---|---:|
| Tick 7.000 | Chơi được khoảng 7 giây rồi nhận Pause | Khoảng 7.000 ms |
| Tick 17.000 | Đã Pause khoảng 10 giây, vòng lặp vẫn cập nhật mốc phục vụ | Vẫn khoảng 7.000 ms |
| Tick 17.000, nhận Resume | `running_elapsed_ms=0` của lượt bắt đầu từ PAUSED | Giữ nguyên |
| Khoảng tick 20.000 | Chơi thêm khoảng 3 giây | Đến chu kỳ thử sinh |

Các mốc chỉ minh họa. Trạng thái được đổi theo vòng lặp: lượt nhận Pause có thể đã tính một khoảng nhỏ trước khi chuyển trạng thái nhưng sau đó không gọi bộ vật cản, nên có sai số cỡ lượt phục vụ tại ranh giới. Code không đo Pause ở mức thời điểm cạnh nút phần cứng tuyệt đối.

### 2.8. Resume chờ đủ một chu kỳ di chuyển mới

Vì `last_move_ms` được đặt lại khi trạng thái trở thành RUNNING, điều kiện di chuyển trong cùng lượt Resume có chênh lệch bằng 0 và chưa đạt chu kỳ.

Ví dụ NORMAL cần 300 ms/ô: dù trước Pause đã gần đến bước tiếp theo, Resume sẽ đợi một chu kỳ mới khoảng 300 ms. HARD tương tự với 250 ms. Chương trình không giữ phần dư của bộ đếm bước đi như cách giữ phần tích lũy sinh vật cản.

Do đó phải phân biệt hai đồng hồ:

| Mốc | Hành vi khi Resume |
|---|---|
| `last_move_ms` của bước đi | Đặt lại để chờ đủ một chu kỳ mới |
| `elapsed_ms` của sinh vật cản | Giữ tích lũy trước Pause, chỉ cộng RUNNING tiếp theo |

### 2.9. Hiển thị PAUSED trên Nokia và LCD

`game_render()` vẫn vẽ map, vật cản, mồi và thân đang giữ nguyên. Ở cuối hàm có:

```c
if (state==SNAKE_PAUSED) display_text(0,0,paused_text);
```

`paused_text` là `PAUSED`. Khi Resume, một lần vẽ mới xóa nội dung cũ và không vẽ chữ Pause nữa.

LCD được `lcd_for_state()` đưa qua `lcd1602_show_obstacles()` cho cả RUNNING và PAUSED, với cờ Pause truyền vào:

```c
void lcd1602_show_obstacles(GameLevel level,MapId map,
                            unsigned char obstacles,unsigned char paused)
{
    show_game_line(level,map);
    if (paused) copy_flash(16,obst_paused);
    else copy_flash(16,obst_playing);
    desired[21]=(char)('0'+obstacles/10u);
    desired[22]=(char)('0'+obstacles%10u);
}
```

LCD giữ Level/Map và số thanh, đổi dòng trạng thái sang `OBST:xx PAUSED`. LED 7 đoạn tiếp tục quét và giữ điểm; Timer1 vẫn đếm mili giây. Giữ điểm không đồng nghĩa tắt LED.

### 2.10. Restart khi đang Pause

Nhánh phím 9 nằm trước nhánh Pause trong `snake_handle_key()`. Vì vậy nó vẫn gọi `snake_init()` khi PAUSED, đưa rắn về 3 đốt, điểm 0, hướng RIGHT, xóa thanh sinh thêm và tạo mồi mới, giữ Level/Map.

`main.c` đặt lại các mốc và cờ xử lý kết quả cho ván mới. Đây là khởi tạo lại ván, khác Resume vốn giữ thân, điểm, mồi và vật cản. Phím 1 chỉ đưa về menu sau GAME OVER/WIN; bản hiện tại không dùng phím 1 để rời ván đang Pause.
