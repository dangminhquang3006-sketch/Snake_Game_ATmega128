# Tính năng chết và kết thúc ván — GAME OVER

## 1. Ví dụ dễ hình dung

### Ví dụ 1: Đầu rắn đi vào thân

Giả sử rắn đang có 8 đốt, đầu ở `(5,3)`, đi sang phải. Thân đã uốn thành hình sau:

```text
                    Cột x
                 4     5     6

Hàng y = 2       .     T     o
Hàng y = 3       o     H →   o  ← ô đầu sắp tới
Hàng y = 4       o     o     o

H: đầu rắn       o: thân rắn       T: đuôi
```

Mảng tọa độ theo thứ tự từ đầu đến đuôi:

```text
[(5,3), (4,3), (4,4), (5,4), (6,4), (6,3), (6,2), (5,2)]
```

Đầu sắp đi tới `(6,3)`, nhưng ô này đang là đốt ở chỉ số 5. Đó là một đốt thân không rời ô trong bước này, nên chương trình chuyển trạng thái sang **GAME OVER**.

| Dữ liệu | Trước bước va chạm | Sau khi phát hiện va chạm |
|---|---|---|
| Trạng thái | `SNAKE_RUNNING` | `SNAKE_GAME_OVER` |
| Đầu trong mảng | `(5,3)` | Vẫn `(5,3)` |
| Ô đầu dự kiến | `(6,3)` | Bị từ chối |
| Chiều dài | 8 | Vẫn 8 |
| Điểm, giả sử đang có 80 | 80 | Vẫn 80 |

Chương trình phát hiện va chạm **trước khi dịch mảng rắn**. Nó không cần cho đầu đi vào thân rồi mới báo chết. Sau đó Nokia chuyển sang màn kết quả:

```text
GAME OVER
SCORE: 0080
9:RESTART
1:MENU
```

### Ví dụ 2: Đâm vào tường hoặc thanh chắn

Trên map PILLARS, ô `(4,2)` là tường cố định. Nếu đầu đang ở `(3,2)` đi sang phải thì ô sắp tới là `(4,2)` và ván kết thúc.

Tương tự, nếu một thanh chắn đã tồn tại ở các ô `(8,5)`, `(9,5)`, `(10,5)`, đầu đang ở `(7,5)` đi sang phải sẽ đâm vào ô `(8,5)` và chết. Đây là thanh đã sinh trước đó, rắn đang di chuyển tới gần nó.

### Ví dụ 3: Qua cạnh màn hình không đồng nghĩa với chết

Cả NORMAL và HARD hiện đều cho xuyên biên. Đầu ở `(13,4)` đi sang phải sẽ xuất hiện tại `(0,4)`.

- Nếu `(0,4)` là ô có thể đi vào thì rắn tiếp tục.
- Nếu `(0,4)` có tường, vật cản hoặc đốt thân không rời ô thì rắn chết do va chạm tại ô đó.

Rắn không có số mạng hay thanh máu. Một lần va chạm bị xác định là không hợp lệ sẽ kết thúc ván hiện tại. Người chơi nhấn **9** để chơi lại hoặc **1** để về menu chọn Level.

## 2. Cách chương trình thực hiện

Logic phát hiện chết nằm trong [game/game.c](../game/game.c). [main.c](../main.c) điều phối việc dừng gameplay, xử lý kết quả, cập nhật màn hình và nhận phím sau khi kết thúc. Các khối C dưới đây được trích từ source hiện tại.

### 2.1. Biểu diễn trạng thái chạy, chết và thắng

Trong [game/snake_game.h](../game/snake_game.h), trạng thái rắn được khai báo:

```c
typedef enum {
    SNAKE_RUNNING=0,
    SNAKE_GAME_OVER,
    SNAKE_WIN
} SnakeState;
```

| Trạng thái | Giá trị enum | Ý nghĩa |
|---|---:|---|
| `SNAKE_RUNNING` | 0 | Ván đang chạy, được di chuyển và sinh vật cản |
| `SNAKE_GAME_OVER` | 1 | Đã chết, ván kết thúc |
| `SNAKE_WIN` | 2 | Đã thắng, ván kết thúc |

Biến `state` lưu trạng thái hiện tại, còn `snake_state()` cho các phần khác đọc nó:

```c
static SnakeState state;
```

```c
SnakeState snake_state(void) { return state; }
```

Cần phân biệt hai biến trạng thái trong chương trình:

| Biến | Phạm vi điều khiển |
|---|---|
| `app_state` trong `main.c` | Đang ở menu Level, menu Map hay màn chơi |
| `state` trong `game.c` | Rắn đang chạy, chết hay thắng |

Khi chết, `app_state` vẫn ở `APP_STATE_GAME` để hiển thị kết quả và nhận lựa chọn sau ván; `state` đổi sang `SNAKE_GAME_OVER`. Chương trình chỉ về menu khi người chơi nhấn phím 1.

### 2.2. Các trường hợp nào trong code đặt GAME OVER?

Trong `snake_step()`, có bốn vị trí gán `state=SNAKE_GAME_OVER`:

| Nhánh | Điều kiện | Vai trò |
|---|---|---|
| Kiểm tra biên | Tọa độ đầu vẫn nằm ngoài lưới sau xử lý xuyên biên | Bảo vệ tọa độ; đi qua cạnh bình thường hiện được xử lý bằng xuyên biên |
| Kiểm tra bản đồ | Ô đầu dự kiến có tường cố định hoặc vật cản sinh thêm | Luật thua do va chạm vật cản |
| Kiểm tra thân | Ô đầu dự kiến trùng phần thân phải kiểm tra | Luật thua do tự đâm vào thân |
| Sau đảo trong HARD | Không xác định được hướng từ đầu và cổ mới | Bảo vệ dữ liệu hình học của rắn |

Các trường hợp thua thường gặp khi chơi là đâm vào thân, tường hoặc thanh chắn. Nhánh kiểm tra biên và nhánh không tìm được hướng sau đảo là các kiểm tra bảo vệ; không có nghĩa là cứ chạm cạnh hoặc cứ đảo đầu là chết.

Chương trình không có luật chết do hết thời gian trong bản hiện tại. Chu kỳ 10 giây là để thử sinh vật cản, không phải đồng hồ đếm ngược mạng sống.

### 2.3. Tính ô đầu dự kiến và xử lý xuyên biên trước

Đầu `snake_step()`:

```c
if (turn_queued) direction=next_direction;
turn_queued=0;
new_x=(signed int)segments[0].x;
new_y=(signed int)segments[0].y;
if (direction==DIR_UP) --new_y;
else if (direction==DIR_DOWN) ++new_y;
else if (direction==DIR_LEFT) --new_x;
else ++new_x;

wrap_position(&new_x,&new_y);
```

Hàm dùng hướng rẽ chờ nếu có, xóa cờ chờ rồi lấy tọa độ đầu hiện tại. Các biến `new_x`, `new_y` là tọa độ tạm, chưa được ghi vào `segments[0]`.

| Hướng | Thay đổi tọa độ dự kiến |
|---|---|
| UP | `new_y` giảm 1 |
| DOWN | `new_y` tăng 1 |
| LEFT | `new_x` giảm 1 |
| RIGHT | `new_x` tăng 1 |

Hai biến có kiểu `signed int` để biểu diễn được `-1` khi đi qua cạnh trái hoặc trên. Sau đó `wrap_position()` xử lý biên:

```c
static void wrap_position(signed int *x,signed int *y)
{
    if (!snake_can_wrap()) return;
    if (*x<0) *x=GRID_COLS-1;
    else if (*x>=GRID_COLS) *x=0;
    if (*y<0) *y=GRID_ROWS-1;
    else if (*y>=GRID_ROWS) *y=0;
}
```

Với lưới 14×8:

| Tọa độ vượt cạnh | Tọa độ sau xử lý |
|---|---|
| `x=-1` | `x=13` |
| `x=14` | `x=0` |
| `y=-1` | `y=7` |
| `y=8` | `y=0` |

`snake_can_wrap()` trả về đúng cho cả NORMAL và HARD. Vì mỗi bước chỉ đi một ô, các trường hợp vượt cạnh bình thường được đưa lại vào lưới trước khi kiểm tra va chạm.

### 2.4. Kiểm tra tọa độ còn ngoài lưới hay không

Sau bước xuyên biên:

```c
if (new_x<0 || new_x>=GRID_COLS || new_y<0 || new_y>=GRID_ROWS) {
    state=SNAKE_GAME_OVER;
    return 1;
}
```

`||` nghĩa là chỉ cần một trong các điều kiện đúng thì tọa độ không hợp lệ. Trong một ván bình thường với hai Level hiện có, việc đi qua cạnh không làm nhánh này được kích hoạt vì `wrap_position()` đã xử lý.

Nếu vẫn gặp tọa độ ngoài lưới, chương trình đặt GAME OVER và trả về ngay để không dùng tọa độ đó truy cập dữ liệu bản đồ hoặc thân rắn.

Hai thao tác có vai trò khác nhau:

- `state=SNAKE_GAME_OVER` lưu việc ván đã kết thúc.
- `return 1` kết thúc lần gọi `snake_step()` và báo cho `main.c` rằng cần vẽ lại.

Giá trị trả về 1 không có nghĩa rắn còn sống hoặc vừa ăn điểm. Trạng thái sống/chết phải đọc từ `snake_state()`.

### 2.5. Kiểm tra tường cố định và vật cản sinh thêm

```c
if (game_map_is_blocked(new_x,new_y) ||
    dynamic_obstacle_is_blocked(new_x,new_y)) {
    state=SNAKE_GAME_OVER;
    return 1;
}
```

Chỉ cần một trong hai hàm trả về 1 thì ván kết thúc.

**Tường cố định** được tra qua:

```c
unsigned char game_map_is_blocked(signed int x, signed int y)
{
    if (x<0 || x>=GRID_COLS || y<0 || y>=GRID_ROWS) return 1;
    return blocked_at(current_map,(unsigned char)x,(unsigned char)y);
}
```

`blocked_at()` đọc bit tương ứng với ô `(x,y)` trong mảng tường của map hiện tại. Map CLASSIC không có tường cố định; PILLARS và LABYRINTH có các ô tường được định nghĩa sẵn.

**Vật cản sinh thêm** được tra qua:

```c
unsigned char dynamic_obstacle_is_blocked(signed int x,signed int y)
{
    if (x<0 || x>=GRID_COLS || y<0 || y>=GRID_ROWS) return 1;
    return (unsigned char)((blocked_rows[(unsigned char)y]>>x)&1u);
}
```

`blocked_rows[y]` là mặt nạ bit của hàng `y`. Dịch sang phải `x` bit rồi `&1u` lấy đúng bit của cột cần xét. Bit bằng 1 nghĩa là ô đó thuộc một thanh chắn.

Ví dụ thanh ở cột 8, 9, 10 của hàng 5 khiến `blocked_rows[5]` có ba bit tương ứng được bật. Đầu đi vào bất kỳ ô nào trong ba ô đó đều thua, không chỉ ô đầu bên trái của thanh.

Hai hàm cũng coi tọa độ ngoài lưới là bị chặn. Trong luồng `snake_step()`, tọa độ đã được kiểm tra trước khi tới đây.

### 2.6. Xác định có ăn mồi để chọn phạm vi kiểm tra thân

Nếu qua được biên và tường/vật cản, chương trình tạo `new_head` rồi tính cờ `growing`:

```c
new_head.x=(unsigned char)new_x;
new_head.y=(unsigned char)new_y;
growing=(unsigned char)(new_head.x==food.x && new_head.y==food.y);
```

Cần biết có ăn hay không vì đuôi rắn có hai cách xử lý:

- Không ăn: đầu tiến một ô và đuôi cũ rời ô trong cùng bước.
- Có ăn: đầu tiến một ô nhưng đuôi được giữ lại để chiều dài tăng một đốt.

Do đó, đi vào ô đuôi cũ ở bước không ăn có thể hợp lệ, còn đuôi không được bỏ qua trong bước có ăn. Mồi hợp lệ luôn nằm ngoài thân, nhưng nhánh kiểm tra vẫn thể hiện đầy đủ quy tắc này.

### 2.7. So sánh đầu dự kiến với các đốt thân

```c
collision_limit=growing ? current_length : (unsigned char)(current_length-1);
for (i=1; i<collision_limit; ++i) {
    if (new_head.x==segments[i].x && new_head.y==segments[i].y) {
        state=SNAKE_GAME_OVER;
        return 1;
    }
}
```

Ý nghĩa từng biến:

| Biến | Vai trò |
|---|---|
| `current_length` | Số đốt đang dùng trong `segments[]` |
| `growing` | Có ăn mồi ở bước này không |
| `collision_limit` | Giới hạn trên của vòng kiểm tra, không bao gồm chỉ số này |
| `i` | Chỉ số đốt đang được so sánh với đầu dự kiến |
| `new_head` | Ô đầu muốn đi tới, đã xử lý xuyên biên |

Vòng lặp bắt đầu từ `i=1`, không so sánh với phần tử 0 là đầu cũ. Mỗi lượt phải trùng cả `x` và `y` mới là trùng ô.

Với ví dụ 8 đốt ở phần đầu và bước không ăn:

```text
current_length  = 8
collision_limit = 7
Các chỉ số xét  = 1, 2, 3, 4, 5, 6
Đuôi chỉ số 7   = bỏ qua vì sẽ rời ô
```

Đầu dự kiến `(6,3)` trùng `segments[5]`, nên nhánh trong vòng lặp đặt GAME OVER rồi trả về. Không cần xét nốt đốt 6 sau khi đã phát hiện một va chạm.

### 2.8. Vì sao đi vào ô đuôi đang rời đi có thể không chết?

Giả sử rắn có 4 đốt:

```text
segments = [(5,3), (4,3), (4,4), (5,4)]
             đầu                       đuôi
```

Đầu ở `(5,3)` vừa nhận lệnh rẽ DOWN, ô sắp tới là `(5,4)`, đúng vị trí đuôi. Nếu ô này không có mồi:

```text
growing         = 0
collision_limit = current_length-1 = 3
Vòng kiểm tra   = i=1, i=2
```

Đuôi ở chỉ số 3 không bị so sánh. Sau bước dịch mảng không ăn:

```text
[(5,4), (5,3), (4,3), (4,4)]
   đầu                       đuôi mới
```

Các đốt vẫn không trùng nhau. Tọa độ đuôi cũ trở thành đầu mới đúng lúc đuôi cũ rời đi, nên bước này hợp lệ.

Nếu luôn kiểm tra hết mọi đốt kể cả đuôi ở bước không ăn, chương trình sẽ báo chết sai. Ngược lại, nếu luôn bỏ qua đuôi ở bước ăn, kiểm tra sẽ không phản ánh việc đuôi được giữ lại. Đó là lý do cần `collision_limit` thay đổi theo `growing`.

Quy tắc này cũng áp dụng khi ô đầu dự kiến được tìm thấy qua cạnh màn hình: xuyên biên không bỏ qua kiểm tra thân, nó chỉ đổi tọa độ trước khi kiểm tra.

### 2.9. Kiểm tra bảo vệ sau khi đảo đầu–đuôi trong HARD

Sau một bước ăn hợp lệ, nếu chưa thắng và đang ở HARD, chương trình đảo thứ tự đốt. Sau đó nó kiểm tra:

```c
if (!direction_from_neck(&direction)) {
    state=SNAKE_GAME_OVER;
    return 1;
}
```

`direction_from_neck()` đọc đầu mới và cổ mới để chọn hướng đi ra xa cổ, có xét các cặp đốt kề qua cạnh. Hàm trả về 0 nếu không tìm được quan hệ đầu–cổ hợp lệ, ví dụ thiếu đốt hoặc hai tọa độ không kề nhau theo quy tắc đang dùng.

Trong trạng thái rắn hợp lệ, hai đốt liên tiếp vẫn kề nhau sau khi đảo, nên đảo đầu–đuôi không tự làm người chơi chết. Nhánh trên ngăn chương trình tiếp tục chạy khi dữ liệu không đủ để xác định hướng.

Khác với va chạm tường/thân, kiểm tra này nằm **sau** đoạn tăng chiều dài, cộng điểm và đảo mảng. Nếu nó thất bại, các thay đổi đó đã diễn ra. Vì vậy, không nên hiểu rằng mọi nhánh GAME OVER đều giữ nguyên hoàn toàn dữ liệu như trước bước di chuyển.

Chi tiết xác định hướng được trình bày trong [DAO_DAU_DUOI_KHI_AN_MOI.md](DAO_DAU_DUOI_KHI_AN_MOI.md).

### 2.10. Va chạm thông thường kết thúc bước trước khi thay đổi thân và điểm

Trong `snake_step()`, thứ tự các nhóm xử lý là:

```text
Dùng hướng rẽ đang chờ
  → tính tọa độ đầu sắp tới
  → xuyên biên
  → kiểm tra biên
  → kiểm tra tường/vật cản
  → tính growing
  → kiểm tra thân
  → dịch mảng, tăng chiều dài/điểm nếu ăn
  → gán đầu mới
  → kiểm tra WIN
  → đảo nếu HARD vừa ăn, kiểm tra hướng sau đảo
  → sinh mồi mới nếu vừa ăn và chưa kết thúc
```

Nếu chết ở các bước kiểm tra biên, tường hoặc thân, lệnh `return 1` khiến chương trình không chạy tới đoạn dịch mảng, cộng điểm hoặc sinh mồi mới.

Ví dụ điểm đang là 80 và đầu dự kiến đâm vào tường: điểm cuối ván vẫn là 80, chiều dài giữ nguyên và tọa độ đầu trong mảng vẫn là ô cuối cùng hợp lệ. Tuy nhiên, hướng rẽ đang chờ có thể đã được áp dụng ở đầu hàm; việc giữ nguyên ở đây nói về thân, chiều dài và điểm của nhánh va chạm sớm.

### 2.11. `main.c` nhận kết quả chết và yêu cầu cập nhật màn hình

Phần gọi bước di chuyển:

```c
if (app_state==APP_STATE_GAME && snake_state()==SNAKE_RUNNING &&
    (unsigned long)(now_ms-last_move_ms)>=move_period_ms) {
    last_move_ms=now_ms;
    redraw=snake_step();
    if (snake_state()!=SNAKE_RUNNING) {
        update_lcd=1;
    }
}
```

Khi `snake_step()` phát hiện chết:

1. Nó đặt `state=SNAKE_GAME_OVER`.
2. Nó trả về 1, nên `redraw=1`.
3. `main.c` đọc trạng thái mới, thấy không còn RUNNING nên đặt `update_lcd=1`.
4. Phần cuối vòng lặp gọi `game_render()` để vẽ kết quả và `lcd_for_state()` để chuẩn bị nội dung LCD.

`redraw` và `update_lcd` là các cờ xử lý của lượt vòng lặp hiện tại. Chúng không lưu trạng thái chết lâu dài; biến `state` mới giữ thông tin ván đã kết thúc.

### 2.12. Vì sao rắn dừng nhưng chương trình vẫn nhận được phím?

Có hai lớp ngăn tiếp tục di chuyển:

- Điều kiện gọi trong `main.c` yêu cầu `snake_state()==SNAKE_RUNNING`.
- Ngay đầu `snake_step()` cũng có kiểm tra:

```c
if (state!=SNAKE_RUNNING) return 0;
```

Sau GAME OVER, lần lặp tiếp theo không gọi bước di chuyển. Ngay cả khi một nơi khác gọi trực tiếp `snake_step()`, kiểm tra trong hàm cũng từ chối xử lý.

Sinh vật cản cũng chỉ được gọi khi rắn RUNNING. Bên trong `dynamic_obstacle_update()` còn có kiểm tra trạng thái riêng. Vì vậy, sau khi chết không có thanh mới sinh thêm và không tiếp tục tích lũy thời gian đang chơi cho bộ sinh vật cản.

Vi điều khiển không bị tắt, không reset và không mắc trong một vòng chờ chết riêng. Vòng `while (1)` của chương trình vẫn tiếp tục đọc Timer, quét keypad, quét LED 7 đoạn và phục vụ LCD. Nhờ vậy phím 9 và phím 1 vẫn hoạt động.

### 2.13. Hiển thị GAME OVER trên Nokia 5110

Ở đầu `game_render()`, chương trình xóa màn hình. Sau đó ưu tiên kiểm tra trạng thái kết thúc:

```c
if (state==SNAKE_GAME_OVER) {
    display_text(0,4,over_text);
    render_final_score();
    return;
}
```

Nếu đã chết, nó in `GAME OVER`, gọi `render_final_score()` rồi `return`. Vì đã trả về, phần vẽ bản đồ, mồi và thân rắn ở phía sau không được chạy. Màn hình được thay bằng kết quả, không chỉ đóng băng hình ván đang chơi.

Hàm hiển thị điểm và hướng dẫn:

```c
static void render_final_score(void)
{
    unsigned int value=snake_get_score();
    if (value>9999u) value=9999u;
    score_text[7]=(char)('0'+value/1000);
    score_text[8]=(char)('0'+(value/100)%10);
    score_text[9]=(char)('0'+(value/10)%10);
    score_text[10]=(char)('0'+value%10);
    display_text_ram(0,18,score_text);
    display_text(0,31,restart_text);
    display_text(0,40,menu_text);
}
```

`score_text` có giá trị ban đầu `SCORE: 0000`. Các vị trí 7, 8, 9, 10 lần lượt nhận hàng nghìn, trăm, chục và đơn vị.

Ví dụ điểm 80 được tách thành `0`, `0`, `8`, `0`, tạo chuỗi `SCORE: 0080`. Mức chặn 9999 chỉ giới hạn dữ liệu đưa ra dạng bốn chữ số, không ghi giá trị này trở lại biến điểm của ván.

| Nội dung | Tọa độ pixel trên Nokia |
|---|---|
| `GAME OVER` | `(0,4)` |
| Điểm cuối ván | `(0,18)` |
| `9:RESTART` | `(0,31)` |
| `1:MENU` | `(0,40)` |

### 2.14. Hiển thị kết quả trên LCD và giữ điểm ở LED 7 đoạn

Hàm `lcd_for_state()` trong `main.c`:

```c
static void lcd_for_state(SnakeState state, GameLevel level, MapId map)
{
    if (state==SNAKE_RUNNING)
        lcd1602_show_obstacles(level,map,dynamic_obstacle_get_count());
    else lcd1602_show_result((unsigned char)(state==SNAKE_WIN),
                             snake_get_score());
}
```

Với GAME OVER, chương trình đi vào nhánh `else`, truyền `won=0` và điểm cuối ván vào:

```c
void lcd1602_show_result(unsigned char won, unsigned int score)
{
    if (score>9999u) score=9999u;
    spaces();
    if (won) copy_flash(0,win_line1);
    else copy_flash(0,over_line1);
    copy_flash(16,score_prefix);
    put_four_digits(23,score);
}
```

LCD 16×2 chuyển sang hai dòng kết quả, ví dụ:

```text
GAME OVER
SCORE: 0080
```

Thư viện LCD của dự án chuẩn bị nội dung trong bộ đệm rồi cập nhật dần bằng `lcd1602_service()` ở vòng lặp chính.

LED 7 đoạn vẫn hiển thị điểm hiện tại. Va chạm thông thường không đặt `score` về 0, nên điểm cuối ván được giữ cho tới khi chơi lại hoặc về menu. Việc quét LED vẫn tiếp tục dù rắn đã dừng.



### 2.15. Sau khi chết, phím điều hướng và phím 5 được xử lý thế nào?

Trong `snake_handle_key()`, Restart được kiểm tra trước điều kiện chỉ nhận hướng khi RUNNING:

```c
void snake_handle_key(KeyEvent key)
{
    SnakeDirection requested;
    if (key==KEY_RESTART) {
        snake_init();
        return;
    }
    if (state!=SNAKE_RUNNING || turn_queued) return;
    if (key==KEY_UP) requested=DIR_UP;
    else if (key==KEY_DOWN) requested=DIR_DOWN;
    else if (key==KEY_LEFT) requested=DIR_LEFT;
    else if (key==KEY_RIGHT) requested=DIR_RIGHT;
    else return;
    if (requested==direction || ((requested+2)&3)==direction) return;
    next_direction=requested;
    turn_queued=1;
}
```

Với GAME OVER, các phím hướng bị bỏ qua vì `state!=SNAKE_RUNNING`. Phím 9 vẫn gọi `snake_init()` do nhánh Restart nằm trước điều kiện này. Phím 1 được `main.c` xử lý riêng để về menu.

Phím 5 chỉ dùng xác nhận ở menu. Nó không đổi trạng thái rắn trong ván hoặc màn kết quả. Khi rắn còn RUNNING, yêu cầu quay ngược trực tiếp bị bỏ qua; chết do thân chỉ xảy ra khi ô đầu dự kiến thật sự va vào một đốt phải kiểm tra.

### 2.16. Phím 9 tạo lại ván trên Level và Map hiện tại

Nhánh Restart:

```c
if (key==KEY_RESTART) {
    snake_init();
    return;
}
```

Hàm khởi tạo lại:

```c
void snake_init(void)
{
    dynamic_obstacle_reset();
    segments[0].x=7; segments[0].y=4;
    segments[1].x=6; segments[1].y=4;
    segments[2].x=5; segments[2].y=4;
    current_length=SNAKE_INITIAL_LENGTH;
    score=0;
    food_type=FOOD_CIRCLE;
    direction=DIR_RIGHT;
    next_direction=DIR_RIGHT;
    turn_queued=0;
    state=SNAKE_RUNNING;
    place_food();
}
```

Kết quả sau khi nhấn 9:

| Dữ liệu | Giá trị mới |
|---|---|
| Trạng thái | RUNNING |
| Các đốt ban đầu | `(7,4)`, `(6,4)`, `(5,4)` |
| Chiều dài | 3 |
| Điểm | 0 |
| Hướng hiện tại và hướng chờ | RIGHT |
| Cờ rẽ chờ | 0 |
| Vật cản sinh thêm và thời gian tích lũy của chúng | Xóa, đặt lại |
| Mồi | Chọn lại vị trí và loại khi còn ô trống |
| Level và Map | Giữ lựa chọn hiện tại |

`snake_init()` không gọi hàm đổi Level hay đổi Map, nên người chơi không cần chọn lại. Việc reset vật cản sinh thêm cũng không xóa tường cố định của bản đồ.

Bên `main.c`, nhánh Restart đặt `last_service_ms=now_ms`, `running_elapsed_ms=0` và `last_move_ms=now_ms` để tránh tính thời gian của ván cũ vào bộ sinh vật cản và chờ đủ chu kỳ trước bước di chuyển tiếp theo.


### 2.17. Phím 1 quay về menu Level

Trong `main.c`:

```c
if (key==KEY_MENU &&
    (snake_state()==SNAKE_GAME_OVER || snake_state()==SNAKE_WIN)) {
    app_state=APP_STATE_LEVEL_SELECT;
    game_menu_init(current_level);
    shown_score=0;
    led7seg_set_number(0);
    redraw=1;
    update_lcd=1;
}
```

Điều kiện này chỉ cho về menu từ màn chơi khi đã GAME OVER hoặc WIN. Trong ván đang RUNNING, phím 1 không kích hoạt nhánh về menu này.

`app_state` được đổi sang `APP_STATE_LEVEL_SELECT`, menu chọn lại Level hiện tại và LED đưa về 0. Các cờ yêu cầu vẽ và cập nhật LCD được bật để thay màn kết quả bằng menu.

Lúc vừa về menu, biến trạng thái rắn có thể vẫn là GAME OVER và dữ liệu ván cũ vẫn còn trong bộ nhớ. Chúng không được dùng để tiếp tục chơi vì ứng dụng đang xử lý menu. Khi người chơi chọn Level rồi xác nhận Map, chương trình gọi `snake_init()` để bắt đầu ván mới.

### 2.18. Theo dõi trọn một lần đâm vào thân

Với ví dụ đầu tài liệu, giả sử điểm là 80, mồi nằm ở nơi khác và không có vật cản tại `(6,3)`:

| Thứ tự | Xử lý | Kết quả |
|---:|---|---|
| 1 | `main.c` đến chu kỳ và gọi `snake_step()` | Trạng thái ban đầu là RUNNING |
| 2 | Tính ô đầu đi tới | `new_x=6`, `new_y=3` |
| 3 | Xử lý xuyên biên | Tọa độ đã trong lưới, giữ nguyên |
| 4 | Kiểm tra biên và tường/vật cản | Qua kiểm tra |
| 5 | Tạo `new_head`, so sánh mồi | `growing=0` |
| 6 | Chọn giới hạn kiểm tra thân | `collision_limit=7` |
| 7 | Duyệt các đốt 1 đến 6 | Đến đốt 5 thấy trùng `(6,3)` |
| 8 | Gán trạng thái và trả về | GAME OVER, trả về 1 |
| 9 | `main.c` nhận kết quả | `redraw=1`, `update_lcd=1` |
| 10 | Kiểm tra cập nhật vật cản | Bỏ qua vì không còn RUNNING |
| 11 | Cập nhật hiển thị | Nokia và LCD hiện GAME OVER, điểm 80 |
| 12 | Những vòng lặp tiếp theo | Không di chuyển, vẫn quét phím và LED |

Mảng tọa độ, chiều dài 8 và điểm 80 vẫn được giữ ở nhánh va chạm này. Người chơi lựa chọn nhấn 9 để khởi tạo lại hoặc nhấn 1 về menu; chương trình không tự bắt đầu lại ván ngay sau khi chết.
