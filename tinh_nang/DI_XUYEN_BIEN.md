# Di chuyển xuyên biên màn hình

## 1. Ví dụ dễ hình dung

Màn chơi có 14 cột, từ `x=0` đến `x=13`, và 8 hàng, từ `y=0` đến `y=7`. Khi rắn đi qua một cạnh, đầu xuất hiện ở cạnh đối diện trong cùng bước di chuyển.

| Đầu hiện tại | Hướng | Tọa độ tạm | Đầu sau xuyên biên |
|---|---|---|---|
| `(13,4)` | RIGHT | `(14,4)` | `(0,4)` |
| `(0,4)` | LEFT | `(-1,4)` | `(13,4)` |
| `(6,0)` | UP | `(6,-1)` | `(6,7)` |
| `(6,7)` | DOWN | `(6,8)` | `(6,0)` |

Ví dụ rắn 3 đốt đang đi RIGHT:

```text
Trước bước: [(13,4), (12,4), (11,4)]
              đầu               đuôi

Sau bước, không ăn:
            [(0,4), (13,4), (12,4)]
              đầu               đuôi
```

Đầu ở bên trái, các đốt còn lại vẫn nằm bên phải. Chúng tiếp tục đi qua cạnh lần lượt trong những bước sau. **Cả NORMAL và HARD đều cho xuyên biên** trên cả ba map.

Xuyên biên chỉ đổi vị trí dự kiến. Nếu ô đến ở cạnh đối diện có tường, thanh chắn hoặc phần thân không rời ô, rắn vẫn GAME OVER. Nếu ô đó chứa mồi thì rắn ăn theo luật bình thường.

## 2. Cách chương trình thực hiện

Các hàm dùng chung nằm trong [game/game.c](../game/game.c). Cấu hình lưới nằm trong [config/board_config.h](../config/board_config.h).

### 2.1. Quy ước lưới và hướng

```c
#define GRID_COLS 14
#define GRID_ROWS 8
#define CELL_SIZE 6
```

`GRID_COLS` và `GRID_ROWS` là số ô logic, không phải pixel. Nokia có 84×48 pixel, mỗi ô 6×6 pixel nên khớp lưới 14×8.

`x` tăng sang phải, `y` tăng xuống dưới. Rắn đi một ô theo một trong bốn hướng ở mỗi `snake_step()`; không có bước chéo qua hai cạnh cùng lúc trong một lần cập nhật bình thường.

### 2.2. Quyết định Level nào cho xuyên biên

```c
unsigned char snake_can_wrap(void)
{
    return (unsigned char)(current_level==GAME_LEVEL_NORMAL ||
                           current_level==GAME_LEVEL_HARD);
}
```

Hàm trả về 1 cho NORMAL hoặc HARD. `snake_set_level()` cũng kiểm tra Level và đưa giá trị không hợp lệ về NORMAL, nên hai Level đang dùng đều đi qua cạnh được.

Không có luật “NORMAL xuyên biên, HARD chạm cạnh là chết” trong source hiện tại. Sự khác nhau chính giữa hai Level là chu kỳ di chuyển và đảo đầu–đuôi sau khi ăn trong HARD.

### 2.3. Tính tọa độ tạm bằng kiểu có dấu

Trong `snake_step()`:

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

Đầu hiện tại lưu bằng `unsigned char` vì tọa độ hợp lệ luôn không âm. Khi tính bước tiếp theo, chương trình đổi sang `signed int` để có thể biểu diễn `-1` trước khi xử lý cạnh trái hoặc trên.

Nếu làm phép trừ trực tiếp trên biến tọa độ 8 bit không dấu rồi coi kết quả là tọa độ âm, giá trị có thể trở thành 255. Dùng biến tạm có dấu giữ được ý nghĩa bước đi trước khi quyết định ô đối diện.

Mảng thân vẫn chưa thay đổi ở bước này. Chương trình mới tính ô đầu muốn đi tới.

### 2.4. Đưa tọa độ vượt cạnh về cạnh đối diện

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

Hàm nhận hai con trỏ `x`, `y`. Lệnh `wrap_position(&new_x,&new_y)` truyền địa chỉ biến tạm; phép gán `*x=...` hoặc `*y=...` thay đổi trực tiếp biến của bên gọi.

| Điều kiện | Phép đổi | Ý nghĩa |
|---|---|---|
| `*x<0` | `*x=GRID_COLS-1` | Qua trái thì sang cột cuối |
| `*x>=GRID_COLS` | `*x=0` | Qua phải thì sang cột đầu |
| `*y<0` | `*y=GRID_ROWS-1` | Qua trên thì xuống hàng cuối |
| `*y>=GRID_ROWS` | `*y=0` | Qua dưới thì lên hàng đầu |

Khi tọa độ vẫn trong lưới, hàm không thay đổi nó. Nếu Level không cho xuyên biên thì hàm trả về ngay, để bên gọi xử lý biên; hiện hai Level đều cho phép.

Đây là xử lý dành cho bước di chuyển từng ô. Nó không phải hàm modulo tổng quát cho tọa độ lệch nhiều vòng: chẳng hạn mọi `x>=14` đều được đưa về 0, không lấy `x%14`. Luồng hiện tại chỉ tạo bước lệch một ô nên phù hợp.

### 2.5. Thứ tự xuyên biên và kiểm tra va chạm

```text
Lấy hướng đang dùng
  → tính new_x/new_y có thể vượt lưới
  → wrap_position() đưa về ô đối diện
  → kiểm tra biên còn hợp lệ
  → kiểm tra tường và vật cản tại ô đối diện
  → tính có ăn, kiểm tra thân
  → cập nhật mảng rắn
```

Các hàm `game_map_is_blocked()` và `dynamic_obstacle_is_blocked()` tự xem tọa độ ngoài lưới là bị chặn. Nếu gọi chúng trước bước wrap, một lần qua cạnh hợp lệ có thể bị báo như đâm vào vật cản. Source hiện tại đặt wrap trước các phép kiểm tra này.

Ví dụ đầu `(13,4)` đi RIGHT, code kiểm tra ô `(0,4)`, không đọc bit cột 14. Nếu ô đó bị chặn thì GAME OVER do va chạm tại nơi đến, không phải do cạnh màn hình tự là tường.

### 2.6. Di chuyển thân qua cạnh bằng việc chép tọa độ

Không cần một vòng lặp “dịch từng đốt qua biên” riêng. Vòng cập nhật mảng vẫn chép tọa độ cũ của đốt trước vào đốt sau, rồi đặt `segments[0]=new_head`.

| Bước không ăn | Mảng từ đầu đến đuôi |
|---|---|
| Trước khi đầu qua phải | `[(13,4), (12,4), (11,4)]` |
| Sau bước qua cạnh | `[(0,4), (13,4), (12,4)]` |
| Thêm một bước RIGHT | `[(1,4), (0,4), (13,4)]` |
| Thêm một bước RIGHT | `[(2,4), (1,4), (0,4)]` |

Mọi tọa độ được lưu trong thân vẫn nằm trong giới hạn lưới. Các đốt kề nhau theo luật xuyên biên có thể mang giá trị cột 0 và 13, dù phép trừ số học thông thường cho độ lệch 13.

`game_render()` vẽ từng ô riêng, không vẽ một đường nối trực tiếp từ cột 0 tới 13, nên phần thân có thể hiển thị ở hai mép mà không tô kín cả hàng.

### 2.7. Ăn mồi và đi vào ô đuôi qua cạnh

Sau wrap, phép so sánh `new_head.x==food.x && new_head.y==food.y` dùng chính tọa độ ô đối diện. Nếu mồi tại `(0,4)` và đầu vừa qua từ `(13,4)`, rắn ăn, cộng điểm và giữ đuôi để dài thêm một đốt.

Nếu không ăn, việc đi vào đúng ô đuôi đang rời đi có thể hợp lệ vì kiểm tra thân bỏ qua đuôi của bước không tăng chiều dài. Quy tắc này không thay đổi khi bước đi xuyên biên.

Riêng vượt cạnh không cộng điểm, không tăng chiều dài và không đặt lại thời gian. Chỉ khi nơi đến có mồi mới kích hoạt các thay đổi sau ăn.

### 2.8. Dự đoán ô đầu tiếp theo cũng phải xuyên biên

Bộ sinh vật cản dùng:

```c
void snake_next_head(signed int *x,signed int *y)
{
    SnakeDirection heading=turn_queued ? next_direction : direction;
    *x=(signed int)segments[0].x;
    *y=(signed int)segments[0].y;
    if (heading==DIR_UP) --*y;
    else if (heading==DIR_DOWN) ++*y;
    else if (heading==DIR_LEFT) --*x;
    else ++*x;
    wrap_position(x,y);
}
```

Hàm chọn `next_direction` khi đang có lệnh rẽ chờ, nếu không dùng `direction`. Nó tính tọa độ rồi gọi cùng `wrap_position()` như bước đi thật.

Ví dụ đầu ở `(13,4)` đi RIGHT, ô dự kiến là `(0,4)`. Nếu chỉ dự đoán `(14,4)`, bộ sinh có thể kiểm tra nhầm vị trí. Dùng hàm chung giúp dự đoán và di chuyển thật cùng tuân theo luật cạnh.

Hàm dự đoán chỉ ghi vào hai biến bên gọi truyền tới; nó không dịch `segments[]` và không tiêu thụ cờ rẽ chờ.

### 2.9. Tính khoảng cách an toàn của vật cản qua cạnh

`cell_safe()` tính độ lệch rồi chọn đường ngắn hơn qua cạnh:

```text
dx = min(|x-head.x|, 14-|x-head.x|)
dy = min(|y-head.y|, 8-|y-head.y|)
distance = dx+dy
```

Đầu ở cột 13 và ô đang xét ở cột 0 cùng hàng chỉ cách nhau 1 bước, không phải 13. Vì khoảng cách không quá 2, ô đó bị loại khỏi vị trí sinh thanh mới.

Điều này ngăn thanh xuất hiện quá sát đầu ngay ở cạnh đối diện. Khoảng cách này xét hình học trên lưới có nối cạnh, không phải tìm đường ngắn nhất vòng qua mọi bức tường.

Thanh chắn vẫn phải nằm trọn trong màn hình: đầu trái chỉ từ cột 0 đến 11 cho chiều dài 3 ô. Luật nối cạnh của rắn không làm thanh mới bị vẽ tách đôi ở hai mép.

### 2.10. Liên thông của vật cản và hướng sau đảo

BFS kiểm tra thanh mới có xét nối `(0,y)` với `(13,y)` và `(x,0)` với `(x,7)`. Vì vậy, khi thử một thanh, chương trình vẫn công nhận đường vòng qua cạnh nếu còn đi được.

Ngược lại, bộ kiểm tra map cố định `game_map_validate()` chỉ lan dấu theo hàng xóm trong lưới, không dùng các nối cạnh. Map gốc hiện được yêu cầu liên thông ngay cả trước khi tính đường xuyên biên.

Sau khi đảo đầu–đuôi trong HARD, `direction_from_neck()` cũng có bốn trường hợp kề qua cạnh:

| Cổ mới | Đầu mới | Hướng đi ra xa cổ |
|---|---|---|
| `(13,4)` | `(0,4)` | RIGHT |
| `(0,4)` | `(13,4)` | LEFT |
| `(6,7)` | `(6,0)` | DOWN |
| `(6,0)` | `(6,7)` | UP |

Nhờ vậy, đầu mới tiếp tục theo đoạn đuôi cũ một cách nhất quán, không nhầm hướng chỉ vì tọa độ hai đốt chênh nhau 13 cột hoặc 7 hàng. Cách suy hướng được phân tích trong [DAO_DAU_DUOI_KHI_AN_MOI.md](DAO_DAU_DUOI_KHI_AN_MOI.md).
