# Chiến thắng khi rắn chiếm hết ô chơi — WIN

## 1. Ví dụ dễ hình dung

Giả sử CLASSIC chưa có thanh chắn sinh thêm, nên có 112 ô chơi. Rắn đã dài 111 đốt, chỉ còn một ô chứa mồi. Khi ăn mồi cuối cùng, chiều dài tăng lên 112 và chương trình chuyển sang WIN.

| Dữ liệu | Trước lần ăn cuối | Sau lần ăn cuối |
|---|---|---|
| Số ô chơi hợp lệ | 112 | 112 |
| Chiều dài rắn | 111 | 112 |
| Số ô ngoài thân | 1, đang có mồi | 0 |
| Trạng thái | RUNNING | WIN |
| Mồi tiếp theo | Chưa cần | Không sinh nữa |

Nếu tất cả 109 mồi từ lúc xuất phát đều là mồi tròn +10, điểm cuối là 1090. Nếu đã ăn mồi vuông hoặc sao, điểm có thể cao hơn: **thắng dựa vào chiều dài so với sức chứa, không dựa vào một mốc điểm cố định**.

Ví dụ CLASSIC đã có 2 thanh chắn, mỗi thanh 3 ô, sức chứa còn `112-6=106`. Rắn dài 105 ăn thêm một mồi thì đạt 106 và thắng; không cần dài tới 112.

Nokia hiện `YOU WIN`, điểm và hướng dẫn `9:RESTART`, `1:MENU`. Nhấn 9 để chơi lại cùng Level/Map, hoặc 1 để về menu.

## 2. Cách chương trình thực hiện

Phát hiện WIN nằm trong `snake_step()` và `place_food()` của [game/game.c](game/game.c). [main.c](main.c) tiếp nhận kết quả, dừng cập nhật gameplay và chuẩn bị hiển thị/lưu kỷ lục.

### 2.1. Một trạng thái kết thúc riêng

```c
typedef enum {
    SNAKE_RUNNING=0,
    SNAKE_PAUSED,
    SNAKE_GAME_OVER,
    SNAKE_WIN
} SnakeState;
```

`SNAKE_WIN` khác GAME OVER nhưng cả hai đều là trạng thái ván đã kết thúc. `app_state` vẫn là `APP_STATE_GAME`, để màn kết quả nhận phím Restart/Menu. WIN không tự chuyển sang map khác.

### 2.2. Tính sức chứa theo bản đồ đang chơi

`game_map_get_free_cells()` lấy số ô không có tường cố định. Ban đầu các map có sức chứa:

| Map | Tổng lưới | Tường cố định | Ô không có tường |
|---|---:|---:|---:|
| CLASSIC | 112 | 0 | 112 |
| PILLARS | 112 | 8 | 104 |
| LABYRINTH | 112 | 12 | 100 |

Để trừ thêm các thanh đã sinh, chương trình gọi:

```c
unsigned char dynamic_obstacle_playable_cells(void)
{
    unsigned char x,y,blocked=0;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (dynamic_obstacle_is_blocked(x,y)) ++blocked;
    return (unsigned char)(game_map_get_free_cells()-blocked);
}
```

Hai vòng lặp duyệt từng ô và đếm bit vật cản trong `blocked_rows[]`. Giá trị trả về là số ô không có tường cố định trừ số ô của thanh chắn.

Các thanh được kiểm tra không chồng nhau và không đè lên tường, nên mỗi thanh hợp lệ chiếm thêm đúng 3 ô. Ví dụ PILLARS có 2 thanh thì sức chứa còn `104-6=98`.

### 2.3. Bước ăn phải hợp lệ trước khi được xét thắng

`snake_step()` xử lý hướng, xuyên biên, tường/vật cản và va chạm thân trước. Nếu có va chạm thì đặt GAME OVER và trả về; không chạy tới đoạn tăng chiều dài và kiểm tra WIN.

Với một bước ăn hợp lệ, chương trình giữ đuôi bằng cách dịch mảng từ chỉ số `current_length`, tăng chiều dài một đơn vị, cộng điểm theo loại mồi và gán đầu mới.

Do đó, thắng ở lần ăn cuối vẫn nhận đủ điểm của mồi đó. Mồi tròn +10, vuông +20, sao +50 đều tăng đúng một đốt, xem [AN_MOI_TANG_DIEM_VA_CHIEU_DAI.md](AN_MOI_TANG_DIEM_VA_CHIEU_DAI.md).

### 2.4. So sánh chiều dài sau bước cập nhật

```c
if (current_length==dynamic_obstacle_playable_cells()) state=SNAKE_WIN;
else if (growing) {
    if (current_level==GAME_LEVEL_HARD) {
        reverse_segments();
        if (!direction_from_neck(&direction)) {
            state=SNAKE_GAME_OVER;
            return 1;
        }
        next_direction=direction;
        turn_queued=0u;
    }
    place_food();
}
```

Dòng đầu yêu cầu `current_length==dynamic_obstacle_playable_cells()`. Đây là so sánh bằng, không phải lấy `score` so với một số mục tiêu.

Đọc thứ tự nhánh:

1. Sau khi đầu mới và thân đã cập nhật, tính lại sức chứa.
2. Nếu chiều dài bằng sức chứa, đặt `state=SNAKE_WIN`.
3. Vì có `else if`, nhánh xử lý ăn bên dưới bị bỏ qua khi đã thắng.
4. Chỉ khi chưa thắng và có ăn mới xử lý đảo trong HARD rồi sinh mồi mới.

Điều này đặc biệt quan trọng ở HARD: lần ăn làm đầy bàn chuyển sang WIN ngay, không đảo đầu–đuôi và không cố đặt mồi vào một bàn đã kín.

### 2.5. Ví dụ theo dữ liệu có hai thanh chắn

Giả sử CLASSIC có 2 thanh, rắn đã dài 105, điểm 1020 sau các mồi tròn trước đó, mồi cuối là tròn:

| Bước | Dữ liệu |
|---|---|
| Tính sức chứa | `112-6=106` |
| Đầu đi đúng ô mồi, không va chạm | `growing=1` |
| Dịch mảng giữ đuôi | Dùng thêm phần tử chỉ số 105 |
| Tăng chiều dài | `105+1=106` |
| Cộng điểm | `1020+10=1030` |
| Gán đầu mới | Thân chiếm đủ 106 ô hợp lệ |
| So sánh sức chứa | `106==106`, đặt WIN |
| Sau đó | Không đảo, không sinh mồi, trả về yêu cầu vẽ |

Đây là ví dụ mô tả trạng thái gần cuối ván, không có nghĩa game luôn sinh được đúng hai thanh hoặc người chơi luôn đạt được trạng thái này.

### 2.6. Kiểm tra bổ sung khi không còn chỗ đặt mồi

Đầu `place_food()` đếm ô hợp lệ và xử lý hết ô:

```c
unsigned char x,y,empty_count=0,target;
for (y=0; y<GRID_ROWS; ++y)
    for (x=0; x<GRID_COLS; ++x)
        if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
            !occupied(x,y)) ++empty_count;

if (empty_count==0) {
    state=SNAKE_WIN;
    return;
}
```

Nếu `empty_count==0`, hàm đặt WIN và trả về trước phép lấy dư `%empty_count`. Nhờ đó nó không thực hiện phép chia dư cho 0 hoặc tìm kiếm mồi vô hạn.

Trong luồng ăn bình thường, điều kiện đầy bàn của `snake_step()` đã ưu tiên WIN trước khi gọi `place_food()`. Nhánh trên bảo vệ riêng cho bộ sinh mồi khi được gọi trong trạng thái không còn ô trống.

Một ô trống để đặt mồi phải đồng thời không có tường, vật cản và thân rắn. Kiểm tra đầy bàn vì thế dùng cùng khái niệm ô hợp lệ với phần sinh mồi.

### 2.7. Vật cản không được đặt đè lên thân để tạo chiến thắng giả

Bộ sinh vật cản kiểm tra từng ô của thanh không trùng thân hoặc mồi. Khi còn mồi và đặt được thanh, ô mồi vẫn là một ô chưa bị rắn chiếm.

Việc giảm sức chứa do thêm thanh không phải phép tăng chiều dài rắn. Thanh chỉ giảm số ô cần chiếm cho các bước sau; code không tuyên bố WIN ngay trong hàm sinh vật cản.

`dynamic_obstacle_playable_cells()` được đọc tại bước cập nhật rắn để điều kiện thắng luôn dùng dữ liệu thanh đang tồn tại, thay vì giữ một hằng số 112 từ đầu ván.

### 2.8. Dừng di chuyển và ngừng sinh thêm vật cản

`snake_step()` trả về 1 sau khi đặt WIN để yêu cầu vẽ lại. Trong `main.c`, trạng thái mới không RUNNING làm `update_lcd=1`.

Ở những lượt sau, điều kiện gọi bước đi yêu cầu RUNNING nên không di chuyển nữa. Bên trong hàm còn có kiểm tra `if (state!=SNAKE_RUNNING) return 0;` để bảo vệ khi bị gọi trực tiếp.

Bước cập nhật vật cản nằm sau bước di chuyển và cũng yêu cầu RUNNING. Vì vậy, nếu vừa thắng ở lượt đó, không có thanh mới sinh ở phần sau cùng lượt.

Timer, vòng lặp, keypad và LED vẫn hoạt động. Chỉ tiến triển ván được dừng; MCU không reset khi thắng.

### 2.9. Hiển thị kết quả và điểm

Trong `game_render()`:

```c
if (state==SNAKE_WIN) {
    display_text(0,4,win_text);
    render_final_score();
    return;
}
```

`win_text` là `YOU WIN`. `render_final_score()` ghi điểm bốn chữ số cùng các hướng dẫn phím. Hàm trả về trước phần vẽ bản đồ, nên Nokia hiện kết quả thay cho hình ván đang chạy.

LCD đi qua `lcd_for_state()`, truyền `won=1` vào `lcd1602_show_result()`. Nhánh này hiện `YOU WIN!` và điểm. LED 7 đoạn giữ điểm cuối ván cho tới khi Restart hoặc về menu.

WIN không đồng nghĩa điểm luôn được đặt về 9999 hay một con số cố định. Bộ hiển thị đọc điểm đã được tính từ các lần ăn.

### 2.10. Xử lý kỷ lục và phím sau WIN

Nhánh xử lý kết quả trong `main.c` dùng chung cho WIN và GAME OVER:

```c
if (app_state==APP_STATE_GAME && !result_saved &&
    (snake_state()==SNAKE_GAME_OVER || snake_state()==SNAKE_WIN)) {
    led7seg_blank();
    highscore_record_result(current_level,current_map,MODE_SURVIVAL,
                            snake_get_score());
    led7seg_resume();
    result_saved=1;
}
```

`result_saved` ngăn xử lý kết quả lặp liên tục trong màn cuối. Hàm EEPROM chỉ ghi khi có kỷ lục mới và dữ liệu hợp lệ. Giá trị trả về hiện không được `main.c` dùng để thử lại khi lưu lỗi; chi tiết ở [LUU_KY_LUC_EEPROM.md](LUU_KY_LUC_EEPROM.md).

Phím 5 không đổi WIN thành RUNNING vì Pause/Resume chỉ xử lý hai trạng thái RUNNING và PAUSED. Phím 9 tạo lại ván trên cùng Level/Map; phím 1 về menu Level. Không có bước tự tăng Level hay chuyển map sau khi thắng trong chương trình hiện tại.
