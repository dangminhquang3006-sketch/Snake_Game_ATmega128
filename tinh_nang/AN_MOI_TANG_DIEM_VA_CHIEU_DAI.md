# Tính năng ăn mồi, cộng điểm và tăng chiều dài rắn

## 1. Ví dụ dễ hình dung

Giả sử đang chơi **NORMAL** để quan sát việc dài thêm mà không có bước đảo đầu–đuôi. Rắn ban đầu có 3 đốt, điểm bằng 0, đầu ở `(7,4)` và đang đi sang phải. Ô `(8,4)` có mồi tròn, trị giá 10 điểm.

```text
Trước khi ăn, tất cả các đốt cùng ở hàng y=4:

Cột x:       5       6       7       8
            Đuôi — Thân — Đầu  →   Mồi

Chiều dài: 3 đốt
Điểm:      0

Sau khi đầu tiến vào ô mồi:

Cột x:       5       6       7       8
            Đuôi — Thân — Thân — Đầu →

Chiều dài: 4 đốt
Điểm:      10
```

Điểm quan trọng là **đầu tiến thêm một ô nhưng đuôi không bị bỏ đi trong bước ăn**, nên tổng số đốt tăng từ 3 lên 4. Nếu không ăn, đầu vẫn tiến nhưng đuôi cũ bị bỏ khỏi thân, nên chiều dài giữ nguyên.

| Trạng thái sau bước di chuyển | Mảng tọa độ từ đầu đến đuôi | Chiều dài |
|---|---|---:|
| Trước bước | `[(7,4), (6,4), (5,4)]` | 3 |
| Đi tới `(8,4)` nhưng ô đó không có mồi | `[(8,4), (7,4), (6,4)]` | 3 |
| Ăn mồi tại `(8,4)` | `[(8,4), (7,4), (6,4), (5,4)]` | 4 |

Trong mã nguồn hiện tại, **mỗi lần ăn tăng đúng một đốt**, còn số điểm phụ thuộc loại mồi:

| Loại mồi | Điểm cộng | Số đốt tăng |
|---|---:|---:|
| Hình tròn | 10 | 1 |
| Hình vuông | 20 | 1 |
| Ngôi sao | 50 | 1 |

Ví dụ ăn liên tiếp một mồi tròn, một mồi vuông rồi một ngôi sao:

| Thời điểm | Tổng điểm | Chiều dài |
|---|---:|---:|
| Bắt đầu | 0 | 3 |
| Sau mồi tròn | 10 | 4 |
| Sau mồi vuông | 30 | 5 |
| Sau ngôi sao | 80 | 6 |

Ăn ngôi sao được 50 điểm vẫn chỉ tăng một đốt, không tăng năm đốt. Hai thay đổi **cộng điểm** và **dài thêm** cùng được kích hoạt bởi sự kiện ăn mồi; chương trình không lấy tổng điểm chia cho 10 để tính chiều dài.

Trong HARD, rắn cũng tăng chiều dài và điểm như trên, sau đó còn đảo đầu–đuôi nếu chưa thắng. Chi tiết bước đảo nằm trong [DAO_DAU_DUOI_KHI_AN_MOI.md](DAO_DAU_DUOI_KHI_AN_MOI.md).

## 2. Cách chương trình thực hiện

Phần xử lý chính nằm trong [game/game.c](../../game/game.c), nhất là `snake_step()` và `place_food()`. [main.c](../../main.c) quyết định khi nào rắn di chuyển và cập nhật điểm lên LED 7 đoạn. Các khối C dưới đây được trích từ source hiện tại.

### 2.1. Lưu tọa độ, chiều dài, mồi và điểm

Một đốt rắn được biểu diễn bằng cấu trúc trong [game/snake_game.h](../../game/snake_game.h):

```c
typedef struct {
    unsigned char x;
    unsigned char y;
} SnakePoint;
```

Lưới có 14 cột và 8 hàng. `x` từ 0 đến 13, tăng từ trái sang phải; `y` từ 0 đến 7, tăng từ trên xuống dưới.

Các biến trực tiếp liên quan đến việc ăn mồi là:

```c
static SnakePoint segments[SNAKE_MAX_LENGTH];
static unsigned char current_length;
static SnakePoint food;
static FoodType food_type;
static unsigned int score;
```

| Biến | Ý nghĩa |
|---|---|
| `segments[]` | Mảng tọa độ các đốt theo thứ tự đầu → thân → đuôi |
| `current_length` | Số đốt đang thuộc con rắn, cũng là số phần tử đang dùng trong mảng |
| `food` | Tọa độ ô chứa mồi hiện tại |
| `food_type` | Loại mồi, quyết định hình vẽ và số điểm nhận được |
| `score` | Tổng điểm của ván, không phải chiều dài rắn |

Hai giới hạn trong header:

```c
#define SNAKE_INITIAL_LENGTH 3
#define SNAKE_MAX_LENGTH (GRID_COLS*GRID_ROWS)
```

`SNAKE_MAX_LENGTH` bằng 112. Mảng được dành sẵn cho tối đa 112 đốt, không cần xin thêm bộ nhớ khi rắn ăn. Chỉ các phần tử từ `segments[0]` đến `segments[current_length-1]` được xem là thân hiện tại; phần còn lại chưa được dùng.

Một phép gán như `segments[3]=segments[2]` sao chép cả `x` và `y` của đốt, không phải chỉ sao chép một tọa độ.

### 2.2. Khởi tạo ván với 3 đốt và 0 điểm

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

Các tọa độ ban đầu là:

| Chỉ số | Tọa độ | Vai trò |
|---|---|---|
| 0 | `(7,4)` | Đầu |
| 1 | `(6,4)` | Thân, ngay sau đầu |
| 2 | `(5,4)` | Đuôi |

`current_length=3`, `score=0`, hướng hiện tại và hướng chờ đều là RIGHT. `turn_queued=0` nghĩa là chưa có lệnh rẽ chờ. Trạng thái được đặt thành RUNNING trước khi gọi `place_food()`.

Dòng `food_type=FOOD_CIRCLE` chỉ đặt giá trị ban đầu cho biến. Ngay sau đó, nếu còn ô trống, `place_food()` chọn vị trí và loại mồi mới, nên mồi đầu ván không bị bắt buộc luôn là hình tròn.

Khi nhấn phím 9 chơi lại, `snake_handle_key()` gọi lại `snake_init()`: chiều dài trở về 3, điểm về 0 và các vật cản sinh thêm được xóa.

### 2.3. Chỉ xử lý ăn mồi khi đến bước di chuyển

Trong vòng lặp chính:

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

Chỉ khi đang ở màn chơi, rắn RUNNING và đã đủ chu kỳ, chương trình gọi `snake_step()`. Chu kỳ là 300 ms/ô trong NORMAL và 250 ms/ô trong HARD.

Chạm chuột hoặc bấm phím điều hướng không trực tiếp cộng điểm. Phím chỉ yêu cầu đổi hướng; đến bước di chuyển, đầu rắn phải thật sự đi vào ô mồi thì sự kiện ăn mới xảy ra.

Luồng xử lý chính của một bước là:

```text
Kiểm tra rắn có đang RUNNING
  → dùng hướng rẽ đang chờ nếu có
  → tính vị trí đầu sắp tới và xử lý xuyên biên
  → kiểm tra tường/vật cản
  → xác định có ăn mồi hay không
  → kiểm tra va chạm thân theo trường hợp ăn/không ăn
  → dịch mảng, tăng chiều dài và điểm nếu ăn
  → đưa đầu vào vị trí mới
  → kiểm tra thắng
  → nếu vừa ăn và chưa thắng: xử lý đảo trong HARD, sinh mồi mới
  → cập nhật hình ảnh và điểm
```

`snake_step()` tự kiểm tra trạng thái ở đầu hàm. Nếu đang Pause hoặc ván đã kết thúc, nó trả về 0 và không cập nhật tọa độ, chiều dài hoặc điểm.

### 2.4. Tính vị trí đầu sắp tới trước khi sửa thân rắn

Đầu bước di chuyển, chương trình dùng hướng rẽ đã nhận nếu có rồi tính tọa độ tạm:

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

- UP: giảm `new_y` một đơn vị.
- DOWN: tăng `new_y` một đơn vị.
- LEFT: giảm `new_x` một đơn vị.
- RIGHT: tăng `new_x` một đơn vị.

`new_x` và `new_y` có kiểu có dấu để biểu diễn được `-1` khi đi qua cạnh trên/trái. `wrap_position()` đổi tọa độ vượt cạnh sang cạnh đối diện, vì cả NORMAL và HARD đều cho xuyên biên.

Ví dụ đầu ở `(13,4)` đi RIGHT sẽ tính tạm `(14,4)` rồi chuyển thành `(0,4)`. Nếu mồi ở `(0,4)`, rắn có thể ăn ngay trong bước xuyên biên đó.

Đến đây, `segments[]` vẫn chưa thay đổi. Chương trình phải kiểm tra ô dự kiến trước khi dịch toàn bộ thân.

### 2.5. Kiểm tra tường, vật cản rồi xác định sự kiện ăn

Nếu tọa độ dự kiến ngoài lưới sau bước xử lý biên, hoặc nằm trong tường/vật cản, chương trình chuyển sang GAME OVER và trả về ngay.

Với ô hợp lệ, chương trình tạo tọa độ đầu mới và xác định có ăn hay không:

```c
new_head.x=(unsigned char)new_x;
new_head.y=(unsigned char)new_y;
growing=(unsigned char)(new_head.x==food.x && new_head.y==food.y);
```

Điều kiện gồm hai phép so sánh nối bằng `&&`:

```text
new_head.x == food.x  : cùng cột
new_head.y == food.y  : cùng hàng
Cả hai đúng          : cùng một ô, growing = 1
Ít nhất một sai      : khác ô, growing = 0
```

Ví dụ `new_head=(8,4)` và `food=(8,5)` chỉ cùng cột nên chưa ăn. Đầu phải tới đúng `(8,5)` mới được cộng điểm và dài thêm.

`growing` là cờ cho **bước di chuyển hiện tại**, không phải trạng thái rắn đang dài bao nhiêu. Chương trình tính lại cờ này ở mỗi lần gọi `snake_step()`.

### 2.6. Kiểm tra va chạm thân theo trường hợp ăn và không ăn

```c
collision_limit=growing ? current_length : (unsigned char)(current_length-1);
for (i=1; i<collision_limit; ++i) {
    if (new_head.x==segments[i].x && new_head.y==segments[i].y) {
        state=SNAKE_GAME_OVER;
        return 1;
    }
}
```

Toán tử `? :` chọn phạm vi cần kiểm tra:

- Nếu ăn: `collision_limit=current_length`.
- Nếu không ăn: `collision_limit=current_length-1`.

Vòng lặp bắt đầu ở chỉ số 1 vì chỉ số 0 đang là đầu cũ. Với chiều dài 4:

| Trường hợp | Các chỉ số được kiểm tra | Đuôi cũ ở chỉ số 3 có bị kiểm tra? |
|---|---|---|
| Ăn | 1, 2, 3 | Có, vì đuôi không rời ô |
| Không ăn | 1, 2 | Không, vì đuôi sẽ rời ô trong cùng bước |

Đây là lý do chương trình cần biết `growing` trước khi kiểm tra thân. Nếu đi vào ô đuôi đang rời đi ở một bước không ăn, rắn có thể vẫn di chuyển hợp lệ. Nếu luôn xem đuôi là vật cản, chương trình sẽ báo thua sai trong trường hợp đó.

Nếu phát hiện va chạm thân, hàm kết thúc trước đoạn tăng chiều dài và cộng điểm. Các cập nhật phía sau chỉ diễn ra sau một bước di chuyển hợp lệ.

### 2.7. Khi không ăn: đầu tiến, đuôi cũ bị bỏ khỏi mảng đang dùng

Nhánh không ăn là:

```c
for (i=(unsigned char)(current_length-1); i>0; --i)
    segments[i]=segments[i-1];
```

Sau nhánh này, dòng chung `segments[0]=new_head` đưa đầu vào vị trí mới.

Giả sử `current_length=3`, mảng ban đầu là `[(7,4), (6,4), (5,4)]`, và ô `(8,4)` không có mồi:

| Bước | Phép gán | Mảng đang dùng |
|---|---|---|
| Ban đầu | — | `[(7,4), (6,4), (5,4)]` |
| `i=2` | `segments[2]=segments[1]` | `[(7,4), (6,4), (6,4)]` |
| `i=1` | `segments[1]=segments[0]` | `[(7,4), (7,4), (6,4)]` |
| Gán đầu | `segments[0]=new_head` | `[(8,4), (7,4), (6,4)]` |

Các tọa độ trùng nhau ở giữa bảng chỉ là trạng thái tạm khi đang chép dữ liệu. `game_render()` chỉ đọc sau khi `snake_step()` hoàn tất nên không hiển thị các trạng thái chép dở này.

Tọa độ `(5,4)` không còn thuộc thân. Không có lệnh tăng `current_length` hoặc `score`, nên rắn vẫn có 3 đốt và giữ nguyên điểm.

### 2.8. Khi ăn: giữ đuôi bằng cách mở rộng phần mảng đang dùng

Toàn bộ nhánh tăng chiều dài và điểm, cùng nhánh không ăn để đối chiếu:

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

Điểm khác biệt quan trọng nằm ở chỗ bắt đầu vòng `for`:

| Nhánh | Chỉ số ghi đầu tiên |
|---|---|
| Không ăn | `current_length-1`, là vị trí đuôi hiện tại |
| Có ăn | `current_length`, là phần tử mới ngay sau đuôi hiện tại |

Với 3 đốt đang dùng, chỉ số 0, 1, 2 thuộc thân. Khi ăn, chương trình dùng thêm chỉ số 3:

| Thứ tự | Phép gán | Tác dụng |
|---:|---|---|
| 1 | `segments[3]=segments[2]` | Giữ lại tọa độ đuôi `(5,4)` trong phần tử mới |
| 2 | `segments[2]=segments[1]` | Đốt sau nhận tọa độ `(6,4)` |
| 3 | `segments[1]=segments[0]` | Cổ mới nhận tọa độ đầu cũ `(7,4)` |
| 4 | `++current_length` | Đổi số đốt từ 3 thành 4 |
| 5 | Cập nhật `score` | Cộng điểm theo loại mồi vừa ăn |
| 6 | `segments[0]=new_head` | Đầu mới ở `(8,4)` |

Mảng cuối cùng là:

```text
Chỉ số:       0       1       2       3
Tọa độ:     (8,4)   (7,4)   (6,4)   (5,4)
Vai trò:     Đầu     Thân    Thân    Đuôi
```

Không cần một lệnh riêng để vẽ thêm đốt ở vị trí bất kỳ. Việc giữ lại đuôi cũ, thêm đầu mới và tăng số phần tử đang dùng đã tạo ra con rắn dài thêm một đốt.

### 2.9. Vì sao vòng sao chép phải chạy từ cuối về đầu?

Nếu chép từ đầu về cuối, sau phép `segments[1]=segments[0]`, tọa độ cũ của `segments[1]` đã mất. Phép chép kế tiếp từ phần tử 1 sang phần tử 2 sẽ lấy nhầm tọa độ đầu cũ.

Ví dụ mảng ba tọa độ có tên rút gọn `[A,B,C]`, muốn ăn để thành `[Mới,A,B,C]`:

```text
Chép đúng từ cuối:
[ A, B, C, _ ]
[ A, B, C, C ]  ← giữ C
[ A, B, B, C ]  ← giữ B
[ A, A, B, C ]  ← giữ A
[ Mới, A, B, C ]

Nếu chép sai từ đầu:
[ A, B, C, _ ]
[ A, A, C, _ ]  ← B bị ghi mất
[ A, A, A, _ ]  ← lại sao chép A
[ A, A, A, A ]  ← các đốt bị trùng tọa độ
```

Vì vậy, `i` giảm từ `current_length` xuống 1 để mỗi tọa độ nguồn được đọc trước khi bị ghi đè. Điều kiện `i>0` cũng tránh thực hiện phép đọc `segments[i-1]` với chỉ số âm.

### 2.10. Cộng điểm dựa vào loại mồi, không dựa vào chiều dài

Ba loại mồi trong header:

```c
typedef enum {
    FOOD_CIRCLE=0,
    FOOD_SQUARE,
    FOOD_STAR
} FoodType;
```

Hàm quy đổi loại mồi thành điểm:

```c
static unsigned char food_score(FoodType type)
{
    if (type==FOOD_STAR) return 50u;
    if (type==FOOD_SQUARE) return 20u;
    return 10u;
}
```

`FOOD_CIRCLE=0`, `FOOD_SQUARE=1`, `FOOD_STAR=2` chỉ là mã phân loại. Chúng không trực tiếp là số điểm. Ví dụ `food_type=FOOD_STAR` có giá trị mã là 2 nhưng `food_score(food_type)` trả về 50.

Đoạn cộng điểm nằm trong `if (growing)`:

```c
if (score<=65535u-food_score(food_type))
    score=(unsigned int)(score+food_score(food_type));
else score=65535u;
```

Trong CodeVisionAVR cho ATmega128, `unsigned int` có 16 bit, biểu diễn từ 0 đến 65535. Thay vì cộng trước rồi mới kiểm tra, chương trình kiểm tra còn đủ chỗ cho điểm cộng hay không.

Ví dụ minh họa giới hạn kiểu dữ liệu: nếu `score=65520` và điểm mồi là 50, điều kiện không đạt nên điểm được giữ tại 65535, không bị tràn về một số nhỏ. Đây là cơ chế bảo vệ kiểu dữ liệu; với kích thước bàn chơi hiện tại, điểm của một ván hợp lệ thấp hơn nhiều mức này.

Còn `++current_length` chỉ xuất hiện một lần trong nhánh ăn và không phụ thuộc giá trị `food_score()`. Vì thế, mồi tròn, vuông hay sao đều tăng đúng một đốt.

### 2.11. Vì sao không ghi vượt giới hạn mảng khi rắn gần đầy bàn?

Chương trình chỉ đặt mồi vào ô không có thân rắn, tường hoặc vật cản. Nếu còn mồi hợp lệ để ăn, trước bước ăn phải còn ít nhất một ô chơi chưa bị rắn chiếm.

Vì vậy:

```text
Trước ăn: current_length < số ô chơi hợp lệ <= 112
Chỉ số ghi mới: current_length <= 111
Sau ăn: current_length mới <= 112
```

Mảng `segments[112]` có chỉ số hợp lệ từ 0 đến 111. Trong bước ăn cuối cùng trên CLASSIC không có vật cản, chiều dài trước ăn là 111 nên vòng dịch ghi thêm `segments[111]`, sau đó tăng chiều dài lên 112. Nó không ghi vào `segments[112]`.

Lập luận này dựa trên trạng thái ván hợp lệ và cách `place_food()` sinh mồi. Nhánh tăng chiều dài không có một phép chặn chỉ số riêng; nó dựa vào việc mồi luôn nằm ngoài thân và ván dừng khi đã chiếm hết ô chơi.

### 2.12. Kiểm tra thắng rồi mới xử lý sau ăn

Ngay sau `segments[0]=new_head`, chương trình thực hiện:

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

Thứ tự này quyết định kết quả của lần ăn:

| Điều kiện sau khi tăng chiều dài | Xử lý |
|---|---|
| Chiều dài bằng số ô chơi hợp lệ | Đặt WIN, không sinh mồi mới và không đảo đầu–đuôi |
| Chưa thắng, vừa ăn trong NORMAL | Sinh mồi mới |
| Chưa thắng, vừa ăn trong HARD | Đảo đầu–đuôi, xác định hướng mới rồi sinh mồi mới |
| Không ăn và chưa thắng | Tiếp tục ván, giữ mồi hiện tại |

`dynamic_obstacle_playable_cells()` lấy số ô không có tường cố định của bản đồ rồi trừ số ô vật cản sinh thêm. Ví dụ CLASSIC có hai thanh, mỗi thanh 3 ô, thì số ô hợp lệ là `112-6=106`; rắn thắng khi chiếm đủ 106 ô đó.

Đảo đầu–đuôi trong HARD chỉ đổi thứ tự các tọa độ và hướng đi. Chiều dài và điểm vừa tăng vẫn được giữ nguyên. Phần này được giải thích riêng trong [file đảo đầu–đuôi](DAO_DAU_DUOI_KHI_AN_MOI.md).

### 2.13. Tạo mồi mới trên ô trống sau khi ăn

Sau một lần ăn và nếu chưa kết thúc ván, `place_food()` được gọi:

```c
static void place_food(void)
{
    unsigned char x,y,empty_count=0,target;
    for (y=0; y<GRID_ROWS; ++y)
        for (x=0; x<GRID_COLS; ++x)
            if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y)) ++empty_count;

    if (empty_count==0) {
        state=SNAKE_WIN;
        return;
    }
    target=(unsigned char)(random_next() % empty_count);
    for (y=0; y<GRID_ROWS; ++y) {
        for (x=0; x<GRID_COLS; ++x) {
            if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y)) {
                if (target==0) {
                    food.x=x;
                    food.y=y;
                    /* One PRNG draw per new fruit type. */
                    food_type=snake_food_type_from_roll(
                        (unsigned char)(random_next()%100u));
                    return;
                }
                --target;
            }
        }
    }
}
```

Hàm đi qua lưới hai lần với cùng điều kiện một ô có thể chứa mồi:

```c
!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y)
```

Hàm kiểm tra một ô có thuộc thân không:

```c
static unsigned char occupied(unsigned char x, unsigned char y)
{
    unsigned char i;
    for (i=0; i<current_length; ++i)
        if (segments[i].x==x && segments[i].y==y) return 1;
    return 0;
}
```

**Lượt duyệt thứ nhất: đếm các ô hợp lệ.**

Vòng ngoài chạy các hàng từ 0 đến 7, vòng trong chạy các cột từ 0 đến 13. Mỗi ô không có tường, vật cản hay thân rắn làm `empty_count` tăng một đơn vị.

`occupied()` duyệt đủ `current_length` đốt, nên cả đầu, thân và đuôi đều bị loại khỏi chỗ đặt mồi. Vì lúc này mảng đã cập nhật sau ăn, mồi mới cũng không thể xuất hiện vào đốt vừa được giữ thêm.

Nếu `empty_count==0`, hàm chuyển sang WIN rồi trả về. Đây là kiểm tra bảo vệ của bộ sinh mồi, bổ sung cho phép kiểm tra đầy bàn trong `snake_step()`.

**Chọn thứ tự ô cần lấy.**

```c
target=(unsigned char)(random_next() % empty_count);
```

`random_next()` tạo số giả ngẫu nhiên. Phép `%empty_count` đưa kết quả về khoảng từ 0 đến `empty_count-1`; `target` là thứ tự trong danh sách ô hợp lệ, không phải tọa độ trực tiếp và cũng không phải mã ô trong toàn bộ 112 ô.

**Lượt duyệt thứ hai: tìm đúng ô có thứ tự đã chọn.**

Hàm duyệt lại theo thứ tự hàng rồi cột. Mỗi lần gặp một ô hợp lệ:

- Nếu `target==0`, ghi tọa độ ô đó vào `food` rồi chọn loại mồi.
- Nếu chưa bằng 0, giảm `target` để bỏ qua ô hợp lệ này và tiếp tục.

Ví dụ rút gọn, giả sử chỉ có bốn ô hợp lệ theo thứ tự duyệt:

| Thứ tự | Ô hợp lệ |
|---:|---|
| 0 | `(0,0)` |
| 1 | `(1,0)` |
| 2 | `(4,0)` |
| 3 | `(2,1)` |

Nếu chọn `target=2`, chương trình bỏ qua hai ô đầu và đặt mồi tại `(4,0)`. Nó không thử đi thử lại các tọa độ ngẫu nhiên cho đến khi gặp ô trống; cách đếm rồi chọn thứ tự giúp kết thúc tìm kiếm ngay cả khi rắn đã chiếm gần hết bàn.

### 2.14. Chọn hình dạng mồi và điểm cho lần ăn tiếp theo

Sau khi gán `food.x` và `food.y`, hàm gọi:

```c
food_type=snake_food_type_from_roll(
    (unsigned char)(random_next()%100u));
```

Hàm phân loại:

```c
FoodType snake_food_type_from_roll(unsigned char roll)
{
    if (roll<60u) return FOOD_CIRCLE;
    if (roll<95u) return FOOD_SQUARE;
    return FOOD_STAR;
}
```

| Giá trị đưa vào | Loại mồi | Tỷ lệ thiết kế | Điểm khi ăn |
|---|---|---:|---:|
| 0–59 | Tròn | 60% | 10 |
| 60–94 | Vuông | 35% | 20 |
| 95–99 | Sao | 5% | 50 |

Vị trí mồi và loại mồi dùng hai lượt lấy số giả ngẫu nhiên riêng trong `place_food()`: một lượt chọn thứ tự ô, một lượt chọn loại. Bộ sinh là giả ngẫu nhiên và phép lấy dư không bảo đảm mọi tần suất quan sát trong một ván đúng bằng tỷ lệ thiết kế.

Loại mồi vừa chọn chưa làm điểm tăng ngay. Nó quyết định điểm cho lần sau khi đầu rắn thật sự chạm mồi. Trong lần ăn hiện tại, chương trình đã dùng loại mồi cũ để cộng điểm trước khi gọi `place_food()`, nên không cộng nhầm theo loại mồi mới.

### 2.15. Vẽ thân dài thêm trên Nokia 5110

Sau khi hoàn tất bước di chuyển, `snake_step()` trả về 1 để `main.c` vẽ lại. Trong `game_render()`, vòng vẽ thân là:

```c
for (i=0; i<snake_length(); ++i) {
    point=snake_segment(i);
    display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                      (unsigned char)(point.y*CELL_SIZE),
                      CELL_SIZE-1,CELL_SIZE-1);
}
```

`snake_length()` trả về `current_length`, còn `snake_segment(i)` trả về tọa độ đốt thứ `i`. Sau khi chiều dài tăng từ 3 lên 4, vòng lặp tự chạy thêm một lượt để vẽ đốt thứ tư.

Mỗi ô lưới rộng và cao 6 pixel. Tọa độ `(x,y)` được đổi sang `(x*6,y*6)` trên màn hình; mỗi đốt được tô `5×5` pixel để chừa khe giữa các ô. Ví dụ đầu tại `(8,4)` được vẽ từ điểm ảnh `(48,24)`.

Chương trình xóa vùng hiển thị và vẽ lại theo dữ liệu hiện tại. Nó không kéo dài một hình ảnh rắn cũ độc lập với mảng tọa độ: phần hình ảnh phản ánh `segments[]` và `current_length` sau bước xử lý.

### 2.16. Cập nhật điểm trên LED 7 đoạn và khi kết thúc ván

Trong `main.c`, sau phần xử lý ván:

```c
if (app_state==APP_STATE_GAME && shown_score!=snake_get_score()) {
    shown_score=snake_get_score();
    led7seg_set_number(shown_score);
}
```

`shown_score` lưu điểm đã gửi ra bộ hiển thị. Nếu `snake_get_score()` thay đổi, chương trình lấy điểm mới rồi gọi `led7seg_set_number()`.

Hàm này cập nhật các chữ số cần hiển thị; việc quét LED thực hiện định kỳ qua `led7seg_scan(now_ms)` trong vòng lặp chính. Do đó, tăng điểm không cần một vòng chờ riêng để xuất từng chữ số.

Khi GAME OVER hoặc WIN, Nokia và LCD hiển thị kết quả cuối ván. Việc lưu kỷ lục EEPROM được thực hiện khi ván kết thúc, không phải mỗi lần rắn ăn. Điểm trong `score` là dữ liệu của ván hiện tại và được đặt về 0 khi chơi lại.

### 2.17. Theo dõi trọn một lần ăn từ đầu đến cuối

Với ví dụ NORMAL, mồi tròn tại `(8,4)`, trước bước ăn:

```text
segments       = [(7,4), (6,4), (5,4)]
current_length = 3
score          = 0
food           = (8,4)
food_type      = FOOD_CIRCLE
direction      = DIR_RIGHT
```

Các thay đổi diễn ra theo thứ tự:

| Thứ tự | Xử lý | Kết quả chính |
|---:|---|---|
| 1 | Đến chu kỳ di chuyển, gọi `snake_step()` | Ván phải đang RUNNING |
| 2 | Tính ô đầu sắp tới và xử lý xuyên biên | `new_x=8`, `new_y=4` |
| 3 | Kiểm tra biên, tường và vật cản | Ô dự kiến phải hợp lệ |
| 4 | Tạo `new_head`, kiểm tra trùng mồi | `growing=1` |
| 5 | Kiểm tra thân theo trường hợp ăn | Không được trùng thân hoặc đuôi |
| 6 | Dịch các phần tử 3, 2, 1 từ cuối về đầu | Giữ được tọa độ đuôi `(5,4)` |
| 7 | Tăng chiều dài | `current_length=4` |
| 8 | Cộng điểm theo mồi tròn cũ | `score=10` |
| 9 | Gán `segments[0]=new_head` | Mảng thành `[(8,4), (7,4), (6,4), (5,4)]` |
| 10 | Kiểm tra đầy bàn | Chưa đầy thì tiếp tục |
| 11 | NORMAL bỏ qua đảo, sinh mồi mới | `food` và `food_type` nhận dữ liệu mới |
| 12 | Vẽ lại và cập nhật LED | Hiện rắn 4 đốt và điểm 10 |

Nếu phát hiện va chạm ở bước 3 hoặc 5, hàm chuyển sang GAME OVER và trả về trước bước dịch mảng, tăng chiều dài, cộng điểm.

Ở lần di chuyển sau, nếu không ăn mồi mới, chương trình đi vào nhánh không ăn: chiều dài vẫn 4 và điểm vẫn 10. Vì thế, một mồi đã ăn chỉ làm tăng điểm và chiều dài một lần.
