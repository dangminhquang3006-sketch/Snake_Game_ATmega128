# Tính năng đảo đầu–đuôi khi rắn ăn mồi

## 1. Ví dụ dễ hình dung

Giả sử rắn đang nằm ngang và đi sang phải. Các số dưới đây là tọa độ cột; tất cả các đốt nằm cùng một hàng.

```text
Trước khi ăn:

Cột:       1      2      3      4      5
          Đuôi — Thân — Thân — Đầu →  Mồi

Sau khi tiến vào ô mồi và dài thêm một đốt:

Cột:       1      2      3      4      5
          Đuôi — Thân — Thân — Thân — Đầu

Sau khi đảo đầu–đuôi trong HARD:

Cột:       1      2      3      4      5
        ← Đầu — Thân — Thân — Thân — Đuôi
```

Đầu mới nằm tại cột 1, là vị trí đuôi cũ. Nếu người chơi không rẽ, ở bước di chuyển tiếp theo đầu mới sẽ tiến sang cột 0.

Các ô thân rắn không dịch chuyển thêm trong lúc đảo. Chương trình thay đổi thứ tự các đốt và xác định lại đầu, đuôi, hướng đi. Điểm đã nhận và chiều dài mới đều được giữ nguyên.

## 2. Cách chương trình thực hiện

Phần dưới bám theo mã nguồn hiện tại trong [game/game.c](game/game.c), [main.c](main.c) và [game/snake_game.h](game/snake_game.h). Các khối C trích từ chương trình; các bảng tọa độ là ví dụ để theo dõi dữ liệu.

### 2.1. Chương trình lưu con rắn như thế nào?

Mỗi đốt rắn là một tọa độ trong lưới 14 cột × 8 hàng:

```c
typedef struct {
    unsigned char x;
    unsigned char y;
} SnakePoint;
```

`x` tăng từ trái sang phải, từ 0 đến 13. `y` tăng từ trên xuống dưới, từ 0 đến 7. Vì vậy, đi lên làm `y` giảm, còn đi xuống làm `y` tăng.

Các biến chính của phần xử lý rắn là:

```c
static SnakePoint segments[SNAKE_MAX_LENGTH];
static unsigned char current_length;
static SnakePoint food;
static FoodType food_type;
static unsigned int score;
static SnakeDirection direction;
static SnakeDirection next_direction;
static unsigned char turn_queued;
static SnakeState state;
static GameLevel current_level=GAME_LEVEL_NORMAL;
```

| Biến | Ý nghĩa trong cơ chế đảo đầu–đuôi |
|---|---|
| `segments[]` | Mảng tọa độ các đốt, theo thứ tự đầu → thân → đuôi |
| `current_length` | Số phần tử đang dùng trong mảng; phần tử sau giới hạn này không thuộc thân hiện tại |
| `food`, `food_type` | Vị trí và loại mồi, dùng để phát hiện ăn và tính điểm |
| `score` | Tổng điểm của ván |
| `direction` | Hướng di chuyển hiện tại |
| `next_direction` | Hướng rẽ đã được nhận để dùng ở bước di chuyển tiếp theo |
| `turn_queued` | Bằng 1 khi đã có một lệnh rẽ đang chờ |
| `state` | RUNNING, PAUSED, GAME OVER hoặc WIN |
| `current_level` | NORMAL hay HARD, quyết định có đảo sau khi ăn hay không |

`SNAKE_MAX_LENGTH` bằng `GRID_COLS*GRID_ROWS`, tức 112. Chương trình dành sẵn mảng tối đa 112 đốt, không cấp phát bộ nhớ mới sau mỗi lần ăn. Từ khóa `static` giúp các biến giữ giá trị giữa các lần gọi hàm và chỉ được truy cập trực tiếp trong file triển khai này.

Quy ước quan trọng nhất:

```text
segments[0]                  = đầu
segments[1]                  = cổ, ngay sau đầu
segments[current_length-1]   = đuôi
```

Ví dụ, với mảng `[(4,2), (3,2), (2,2), (1,2)]`, đầu nằm ở `(4,2)` và đuôi nằm ở `(1,2)`. Đảo đầu–đuôi là đảo thứ tự mảng này, đồng thời tính lại hướng di chuyển.

### 2.2. Khi nào hàm xử lý được gọi?

Trong `main.c`, chương trình không di chuyển rắn ở mọi lượt của vòng `while (1)`. Nó kiểm tra đồng hồ và chỉ gọi `snake_step()` khi đủ chu kỳ:

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

Ở HARD, `move_period_ms` là 250 ms. Ở NORMAL, giá trị này là 300 ms. Bản thân `snake_step()` cũng kiểm tra `state`; nếu rắn không ở `SNAKE_RUNNING`, hàm trả về `0` và không xử lý di chuyển.

Giá trị trả về của `snake_step()` là yêu cầu cập nhật hình ảnh: `1` nghĩa là có thay đổi cần vẽ lại, kể cả khi thay đổi đó là GAME OVER. Nó không có nghĩa là người chơi vừa ghi điểm.

Trình tự tổng quát của một bước:

```text
Nhận hướng rẽ đang chờ
  → tính ô đầu sắp tới
  → xử lý xuyên biên
  → kiểm tra tường, vật cản và thân
  → dịch các đốt, tăng chiều dài/điểm nếu ăn
  → gán đầu mới
  → kiểm tra thắng
  → nếu vừa ăn trong HARD và chưa thắng: đảo mảng, tính hướng mới
  → sinh mồi mới nếu vừa ăn và ván chưa kết thúc
  → trả về để main.c vẽ lại
```

### 2.3. Tính vị trí đầu rắn sắp đi tới

Đầu hàm `snake_step()` thực hiện:

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

Nếu `turn_queued` đang bằng 1, hướng đang chờ được đưa vào `direction`. Sau đó cờ được xóa vì lệnh này đã được dùng.

`new_x`, `new_y` là biến tạm có kiểu `signed int`. Kiểu có dấu cho phép biểu diễn `-1` khi đầu rắn đi qua cạnh trái hoặc cạnh trên. Hàm `wrap_position()` đổi tọa độ vượt cạnh sang cạnh đối diện trước khi chương trình kiểm tra va chạm.

Ví dụ, rắn ở `(13,4)` đi sang phải sẽ tính tạm `(14,4)`, rồi được đổi thành `(0,4)`. Bước này chưa sửa `segments[]`; chương trình mới chỉ tính vị trí dự kiến.

### 2.4. Phát hiện ăn mồi và kiểm tra va chạm trước khi đổi dữ liệu

Chương trình kiểm tra biên, tường cố định và vật cản sinh thêm trước. Nếu có va chạm, nó chuyển sang `SNAKE_GAME_OVER` và trả về ngay; không cộng điểm hoặc đảo rắn.

Khi vị trí dự kiến hợp lệ, chương trình tạo `new_head` và kiểm tra trùng mồi:

```c
new_head.x=(unsigned char)new_x;
new_head.y=(unsigned char)new_y;
growing=(unsigned char)(new_head.x==food.x && new_head.y==food.y);
```

`&&` yêu cầu cả tọa độ cột và hàng đều trùng. `growing=1` nghĩa là bước này sẽ ăn mồi và lớn thêm một đốt. Việc kích hoạt dựa trên **vị trí mồi**, không dựa trên một mốc tổng điểm.

Kế tiếp là kiểm tra đầu sắp tới có đâm vào thân không:

```c
collision_limit=growing ? current_length : (unsigned char)(current_length-1);
for (i=1; i<collision_limit; ++i) {
    if (new_head.x==segments[i].x && new_head.y==segments[i].y) {
        state=SNAKE_GAME_OVER;
        return 1;
    }
}
```

`collision_limit` khác nhau giữa hai trường hợp:

| Trường hợp | Đuôi có rời ô hiện tại trong bước này? | Phần thân phải kiểm tra |
|---|---|---|
| Không ăn | Có | Từ chỉ số 1 đến `current_length-2` |
| Có ăn | Không | Từ chỉ số 1 đến `current_length-1`, gồm cả đuôi |

Đi vào đúng ô đuôi cũ có thể hợp lệ khi không ăn, vì đuôi sẽ rời ô đó trong cùng bước. Khi ăn, đuôi được giữ lại nên không được bỏ qua nó trong kiểm tra.

### 2.5. Tăng chiều dài, cộng điểm và đưa đầu vào ô mồi

Sau khi qua các kiểm tra, chương trình cập nhật mảng:

```c
if (growing) {
    /* Food occupies a free cell, so length is below map capacity. */
    for (i=current_length; i>0; --i) segments[i]=segments[i-1];
    ++current_length;
    if (score<=65535u-food_score(food_type))
        score=(unsigned int)(score+food_score(food_type));
    else score=65535u;
} else {
    for (i=(unsigned char)(current_length-1); i>0; --i)
        segments[i]=segments[i-1];
}
segments[0]=new_head;
```

Giả sử trước bước ăn có 4 đốt:

```text
Chỉ số:       0       1       2       3       4
Trước ăn:   (4,2)   (3,2)   (2,2)   (1,2)   chưa dùng
Mồi:        (5,2)
```

Vòng `for` khi ăn chạy từ cuối về đầu:

| Lượt | Phép gán | Tọa độ được chép |
|---|---|---|
| `i=4` | `segments[4]=segments[3]` | `(1,2)` |
| `i=3` | `segments[3]=segments[2]` | `(2,2)` |
| `i=2` | `segments[2]=segments[1]` | `(3,2)` |
| `i=1` | `segments[1]=segments[0]` | `(4,2)` |

Phải chép từ cuối về đầu để không ghi đè tọa độ mà lượt sau còn cần đọc. Sau đó, `++current_length` đổi chiều dài từ 4 thành 5, còn `segments[0]=new_head` đưa `(5,2)` vào đầu mảng.

Kết quả sau khi ăn nhưng **chưa đảo**:

```text
[(5,2), (4,2), (3,2), (2,2), (1,2)]
  đầu                                đuôi
```

Nếu không ăn, vòng dịch bắt đầu ở `current_length-1`, không tăng chiều dài; tọa độ đuôi cũ bị loại khỏi mảng đang dùng.

Điểm được cộng bằng `food_score(food_type)`: mồi tròn +10, mồi vuông +20, ngôi sao +50. Mỗi loại đều chỉ tăng một đốt. Phép kiểm tra `65535u-food_score(food_type)` ngăn biến điểm 16 bit bị tràn; nếu vượt giới hạn biểu diễn thì giữ ở 65535.

### 2.6. Kiểm tra thắng trước khi đảo

Ngay sau khi gán đầu mới, chương trình thực hiện nhánh sau:

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

Cần đọc cấu trúc `if ... else if` theo đúng thứ tự:

1. Nếu chiều dài bằng số ô chơi hợp lệ còn lại, trạng thái trở thành `SNAKE_WIN`. Không đi vào nhánh `else if`, nên không đảo và không sinh mồi mới.
2. Nếu chưa thắng và vừa ăn, chương trình kiểm tra `current_level`.
3. Nếu là HARD, thực hiện đảo và xác định hướng mới.
4. Nếu là NORMAL, bỏ qua khối đảo nhưng vẫn gọi `place_food()`.
5. Nếu không ăn, bỏ qua cả khối xử lý sau ăn.

`dynamic_obstacle_playable_cells()` lấy số ô không có tường cố định rồi trừ các ô vật cản sinh thêm. Vì vậy, điều kiện thắng vẫn khớp khi vùng chơi bị thu hẹp bởi vật cản.

### 2.7. Đảo mảng bằng hai chỉ số chạy từ hai đầu

Hàm được gọi trong nhánh HARD là:

```c
static void reverse_segments(void)
{
    unsigned char left=0u,right=(unsigned char)(current_length-1u);
    SnakePoint saved;
    while (left<right) {
        saved=segments[left];
        segments[left]=segments[right];
        segments[right]=saved;
        ++left;
        --right;
    }
}
```

Ý nghĩa từng biến:

| Biến | Vai trò |
|---|---|
| `left` | Chỉ số phần tử cần đổi chỗ ở phía đầu mảng |
| `right` | Chỉ số phần tử cần đổi chỗ ở phía cuối mảng |
| `saved` | Biến tạm giữ một tọa độ để không mất dữ liệu khi đổi chỗ |

Với mảng 5 đốt sau khi ăn, các lượt diễn ra như sau:

| Thời điểm | `left` | `right` | Nội dung mảng |
|---|---:|---:|---|
| Ban đầu | 0 | 4 | `[(5,2), (4,2), (3,2), (2,2), (1,2)]` |
| Đổi phần tử 0 và 4 | 0 | 4 | `[(1,2), (4,2), (3,2), (2,2), (5,2)]` |
| Đổi phần tử 1 và 3 | 1 | 3 | `[(1,2), (2,2), (3,2), (4,2), (5,2)]` |
| Dừng | 2 | 2 | Phần tử giữa giữ nguyên |

Ba phép gán trong vòng lặp là một lần đổi chỗ:

```text
saved = phần tử bên trái
phần tử bên trái = phần tử bên phải
phần tử bên phải = saved
```

Sau mỗi lần đổi, `++left` và `--right` thu hẹp đoạn chưa xử lý. Điều kiện `left<right` khiến vòng lặp dừng khi hai chỉ số gặp nhau hoặc vượt qua nhau. Với số đốt chẵn, không có phần tử giữa cần giữ riêng.

Hàm không thay đổi `current_length`, `score`, bản đồ hay vật cản. Nó chỉ thay đổi thứ tự các tọa độ đang có. Với `n` đốt, cần `floor(n/2)` lần đổi chỗ, dùng thêm một biến tọa độ `saved`, không cần mảng thứ hai.

### 2.8. Tìm hướng mới bằng đầu và cổ mới

Sau khi đảo, chương trình không thể tiếp tục dùng hướng của đầu cũ. Nó gọi:

```c
if (!direction_from_neck(&direction)) {
    state=SNAKE_GAME_OVER;
    return 1;
}
```

`&direction` là địa chỉ của biến hướng. Hàm nhận địa chỉ đó qua tham số con trỏ `heading`; phép gán `*heading=...` bên trong hàm cập nhật trực tiếp biến `direction` của rắn.

Toàn bộ hàm xác định hướng:

```c
static unsigned char direction_from_neck(SnakeDirection *heading)
{
    SnakePoint head,neck;
    if (current_length<2u) return 0;
    head=segments[0];
    neck=segments[1];
    if (head.y==neck.y) {
        if ((head.x==(unsigned char)(neck.x+1u)) ||
            (head.x==0u && neck.x==GRID_COLS-1u && snake_can_wrap())) {
            *heading=DIR_RIGHT;
            return 1;
        }
        if ((neck.x==(unsigned char)(head.x+1u)) ||
            (head.x==GRID_COLS-1u && neck.x==0u && snake_can_wrap())) {
            *heading=DIR_LEFT;
            return 1;
        }
    } else if (head.x==neck.x) {
        if ((head.y==(unsigned char)(neck.y+1u)) ||
            (head.y==0u && neck.y==GRID_ROWS-1u && snake_can_wrap())) {
            *heading=DIR_DOWN;
            return 1;
        }
        if ((neck.y==(unsigned char)(head.y+1u)) ||
            (head.y==GRID_ROWS-1u && neck.y==0u && snake_can_wrap())) {
            *heading=DIR_UP;
            return 1;
        }
    }
    return 0;
}
```

Các bước đọc hàm:

1. Nếu có ít hơn 2 đốt, không có cổ để suy ra hướng nên trả về `0`. Trong ván bình thường rắn bắt đầu từ 3 đốt và chỉ tăng chiều dài.
2. Lấy `head=segments[0]` và `neck=segments[1]` sau khi đã đảo.
3. Nếu cùng hàng, kiểm tra quan hệ ngang để chọn RIGHT hoặc LEFT.
4. Nếu cùng cột, kiểm tra quan hệ dọc để chọn DOWN hoặc UP.
5. Khi nhận được một quan hệ kề hợp lệ, ghi hướng qua `*heading` và trả về `1`.
6. Nếu không có quan hệ hợp lệ, trả về `0`; bên gọi chuyển sang GAME OVER để không tiếp tục chạy với hướng sai.

Quy tắc là hướng từ cổ đi ra đầu, rồi tiếp tục ra xa cổ:

| Cổ mới | Đầu mới | Hướng được chọn | Ô tiếp theo nếu không rẽ |
|---|---|---|---|
| `(2,2)` | `(1,2)` | LEFT | `(0,2)` |
| `(2,2)` | `(3,2)` | RIGHT | `(4,2)` |
| `(5,4)` | `(5,3)` | UP | `(5,2)` |
| `(5,4)` | `(5,5)` | DOWN | `(5,6)` |

Trong ví dụ mảng 5 đốt ở trên, đầu mới là `(1,2)`, cổ mới là `(2,2)`, nên hướng mới là LEFT.

### 2.9. Vì sao không chỉ đổi RIGHT thành LEFT?

Khi thân uốn cong, đoạn đuôi cũ có thể nằm dọc dù đầu cũ đang đi ngang. Ví dụ:

| Chỉ số | Trước khi ăn, đầu cũ đi RIGHT | Sau khi ăn tại `(8,4)` | Sau khi đảo |
|---|---|---|---|
| 0 | `(7,4)` | `(8,4)` | `(5,5)` — đầu mới |
| 1 | `(6,4)` | `(7,4)` | `(5,4)` — cổ mới |
| 2 | `(5,4)` | `(6,4)` | `(6,4)` |
| 3 | `(5,5)` — đuôi cũ | `(5,4)` | `(7,4)` |
| 4 | Chưa dùng | `(5,5)` | `(8,4)` — đuôi mới |

Đầu mới `(5,5)` nằm dưới cổ `(5,4)`, nên rắn phải đi DOWN. Nếu chỉ đảo RIGHT thành LEFT thì chương trình sẽ chọn sai hướng tiếp tục của đoạn đuôi cũ.

### 2.10. Xử lý đầu và cổ kề nhau qua cạnh màn hình

Do HARD cho xuyên biên, hai đốt liên tiếp có thể có tọa độ rất xa nhau theo cách trừ thông thường. Hàm `direction_from_neck()` có điều kiện riêng cho bốn trường hợp:

| Cổ mới | Đầu mới | Hướng phải chọn | Ô tiếp theo |
|---|---|---|---|
| `(13,4)` | `(0,4)` | RIGHT | `(1,4)` |
| `(0,4)` | `(13,4)` | LEFT | `(12,4)` |
| `(6,7)` | `(6,0)` | DOWN | `(6,1)` |
| `(6,0)` | `(6,7)` | UP | `(6,6)` |

Ví dụ, cổ ở cột 13 và đầu ở cột 0 được hiểu là cổ đi qua cạnh phải để tới đầu. Hướng tiếp tục là RIGHT. Nếu chỉ nhìn `0<13` rồi chọn LEFT thì đầu sẽ quay vào cổ qua cạnh trái ngay ở bước tiếp theo.

### 2.11. Đồng bộ hướng chờ và xử lý phím sau khi đảo

Sau khi tìm được hướng, `snake_step()` gán:

```c
next_direction=direction;
turn_queued=0u;
```

`next_direction` được đưa về hướng mới, và không còn lệnh rẽ chờ áp dụng. Ở đầu bước di chuyển, lệnh rẽ cũ đã được tiêu thụ; hai phép gán này tiếp tục bảo đảm bộ điều khiển nhất quán với đầu mới.

Mã hướng được khai báo theo vòng bốn hướng:

```c
typedef enum { DIR_UP=0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } SnakeDirection;
```

Trong `snake_handle_key()`, quy tắc nhận hướng là:

```c
if (state!=SNAKE_RUNNING || turn_queued) return;
if (key==KEY_UP) requested=DIR_UP;
else if (key==KEY_DOWN) requested=DIR_DOWN;
else if (key==KEY_LEFT) requested=DIR_LEFT;
else if (key==KEY_RIGHT) requested=DIR_RIGHT;
else return;
if (requested==direction || ((requested+2)&3)==direction) return;
next_direction=requested;
turn_queued=1;
```

- Nếu không RUNNING hoặc đã có một lệnh rẽ chờ, không nhận thêm lệnh rẽ.
- Phím 2/8/4/6 được đổi thành UP/DOWN/LEFT/RIGHT.
- Nếu hướng yêu cầu trùng hướng hiện tại, bỏ qua.
- `((requested+2)&3)` lấy hướng đối diện của hướng yêu cầu: UP đối diện DOWN, RIGHT đối diện LEFT. Nếu nó trùng hướng hiện tại, yêu cầu bị loại.
- Một hướng rẽ hợp lệ được ghi vào `next_direction`, còn `turn_queued=1` để dùng ở bước kế tiếp.

Sau khi đảo sang LEFT, phím RIGHT vẫn bị chặn vì nó đưa đầu mới quay vào cổ mới. Người chơi có thể rẽ UP hoặc DOWN nếu không có một lệnh rẽ khác đang chờ. Việc đảo chỉ tạo hướng tiếp tục ban đầu; người chơi vẫn điều khiển được đầu mới.

### 2.12. Sinh mồi mới và hiển thị kết quả đảo

Sau khi nhánh đảo kết thúc thành công, `place_food()` đếm các ô không có tường, vật cản và thân rắn rồi chọn một ô trống để đặt mồi mới. Nếu không còn ô trống, hàm đặt trạng thái WIN.

`snake_step()` trả về `1`, `main.c` đặt yêu cầu vẽ lại và `game_render()` đọc các tọa độ qua `snake_segment(i)`. Các đốt được vẽ từ mảng đã cập nhật.

Trong cách vẽ hiện tại, đầu và thân đều là các ô vuông tô giống nhau. Vì đảo chỉ thay thứ tự đốt, tại đúng thời điểm đảo tập hợp ô đang chiếm vẫn giữ nguyên; dễ nhận thấy kết quả nhất ở bước tiếp theo khi rắn bắt đầu di chuyển từ đuôi cũ.

Không có một bước di chuyển phụ ngay trong `reverse_segments()`. Vòng lặp chính tiếp tục chờ chu kỳ di chuyển; nếu không có lệnh rẽ mới, đầu mới sẽ đi theo hướng vừa suy ra.

Đảo đầu–đuôi không xóa rào chắn hay đặt lại đồng hồ sinh vật cản. Hai cơ chế tiếp tục cùng hoạt động trong HARD. Nếu lần ăn đã làm thắng ván, nhánh WIN được ưu tiên và toàn bộ bước đảo được bỏ qua.
