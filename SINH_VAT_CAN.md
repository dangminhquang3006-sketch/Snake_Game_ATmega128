# Tính năng sinh vật cản theo thời gian

## 1. Ví dụ dễ hình dung

Giả sử người chơi chọn NORMAL hoặc HARD và bắt đầu một ván mới. Ban đầu chưa có vật cản sinh thêm. Sau mỗi **10 giây đang chơi**, chương trình thử đặt thêm **một thanh chắn ngang dài 3 ô** vào vị trí hợp lệ.

Ví dụ trên bản đồ CLASSIC có 14 cột và 8 hàng, sau 10 giây chương trình chọn được vị trí đầu thanh là `(8,5)`:

```text
       Cột x: 01234567890123

Hàng y = 0    ..............
Hàng y = 1    ..............
Hàng y = 2    ..ooH.........
Hàng y = 3    ..............
Hàng y = 4    ..............
Hàng y = 5    ........###...
Hàng y = 6    ...........F..
Hàng y = 7    ..............

H: đầu rắn     o: thân rắn
F: mồi         #: vật cản mới
.: ô trống
```

Thanh chắn chiếm ba ô `(8,5)`, `(9,5)` và `(10,5)`. Rắn phải tránh các ô này; nếu đầu rắn đi vào một ô vật cản thì ván chơi kết thúc.

Nếu mỗi lượt đều tìm được vị trí hợp lệ, số thanh chắn tăng như sau:

| Thời gian đang chơi tích lũy | Số thanh chắn | Số ô bị chiếm bởi vật cản sinh thêm |
|---|---:|---:|
| Bắt đầu | 0 | 0 |
| Khoảng 10 giây | 1 | 3 |
| Khoảng 20 giây | 2 | 6 |
| Khoảng 30 giây | 3 | 9 |
| Khoảng 80 giây | 8 | 24 |

Tối đa có **8 thanh chắn**. Nếu một lượt không tìm được vị trí hợp lệ, chương trình bỏ qua lượt đó và chờ chu kỳ tiếp theo; số thanh chắn không tăng.

Thời gian tạm dừng không được cộng vào chu kỳ. Ví dụ, đã chơi 7 giây rồi Pause thì sau khi Resume cần chơi tiếp khoảng 3 giây mới đến lượt thử sinh vật cản. Các thanh đã sinh giữ nguyên vị trí đến khi chơi lại.

## 2. Cách chương trình thực hiện

Phần xử lý nằm trong [game/game.c](game/game.c), nhóm hàm `dynamic_obstacle`. Vòng lặp trong [main.c](main.c) truyền thời gian và cập nhật màn hình; [drivers/timer.c](drivers/timer.c) cung cấp đồng hồ mili giây. Các đoạn C dưới đây trích từ mã nguồn hiện tại.

### 2.1. Các thông số và quy ước tọa độ

Lưới chơi có 14 cột × 8 hàng, tức 112 ô. Cột `x` chạy từ 0 đến 13; hàng `y` chạy từ 0 đến 7. `y` tăng từ trên xuống dưới.

Chu kỳ trong [config/board_config.h](config/board_config.h):

```c
#define DYNAMIC_OBSTACLE_INTERVAL_MS 10000UL
```

Giới hạn trong [game/game_mode.h](game/game_mode.h):

```c
#define DYNAMIC_OBSTACLE_MAX_COUNT 8u
#define DYNAMIC_OBSTACLE_LENGTH 3u
#define DYNAMIC_OBSTACLE_SAFE_DIST 2u
```

| Thông số | Giá trị | Ý nghĩa |
|---|---:|---|
| Chu kỳ | 10.000 ms | Khoảng thời gian RUNNING giữa hai lượt thử sinh |
| Số thanh tối đa | 8 | Khi đủ giới hạn thì ngừng sinh thêm |
| Chiều dài mỗi thanh | 3 ô | Ba ô cùng hàng, liên tiếp nhau |
| Khoảng cách an toàn | 2 bước | Loại các ô cách đầu rắn không quá 2 bước ngang/dọc, có xét xuyên biên |

Mỗi thanh luôn nằm ngang. Quy tắc này áp dụng cho cả NORMAL và HARD; không cần chọn một chế độ sinh vật cản riêng.

### 2.2. Các biến lưu dữ liệu vật cản

```c
static unsigned int blocked_rows[GRID_ROWS];
static unsigned char queue[GRID_COLS*GRID_ROWS];
static unsigned int visited[GRID_ROWS];
static unsigned char obstacle_count;
/* One left endpoint per horizontal bar; collision uses all occupied cells. */
static SnakePoint obstacle_starts[DYNAMIC_OBSTACLE_MAX_COUNT];
static unsigned long elapsed_ms;
```

| Biến | Dữ liệu lưu | Khi nào dùng? |
|---|---|---|
| `blocked_rows[8]` | Mỗi hàng là một mặt nạ bit đánh dấu các ô vật cản sinh thêm | Kiểm tra đặt vật cản, va chạm và số ô còn chơi được |
| `queue[112]` | Hàng đợi các ô chờ duyệt | Kiểm tra liên thông bằng BFS |
| `visited[8]` | Mặt nạ bit các ô đã được phát hiện trong lần BFS hiện tại | Tránh đưa một ô vào hàng đợi nhiều lần |
| `obstacle_count` | Số thanh đã đặt thành công | Kiểm tra giới hạn 8 thanh và hiển thị |
| `obstacle_starts[8]` | Tọa độ đầu bên trái của mỗi thanh | Vẽ thanh chắn lên Nokia 5110 |
| `elapsed_ms` | Thời gian RUNNING tích lũy kể từ lượt thử gần nhất | Quyết định đã đến chu kỳ 10 giây chưa |

Trong CodeVisionAVR cho ATmega128, `unsigned int` có 16 bit. Một hàng chỉ cần 14 bit nên có thể lưu trong một phần tử. `blocked_rows` và `visited` mỗi mảng chiếm 16 byte; `queue` dùng 112 byte. Hai mảng phục vụ BFS được đặt `static` để không tạo lại chúng trên stack mỗi lần kiểm tra.

Tường có sẵn của map được lưu riêng trong `walls[][]`. `blocked_rows[]` chỉ đánh dấu các vật cản sinh thêm, giúp chơi lại có thể xóa chúng mà vẫn giữ nguyên bản đồ.

### 2.3. Đồng hồ 1 ms đến từ đâu?

Timer1 được cấu hình CTC, chia clock cho 64. Với CPU 8 MHz:

```text
Tần số đếm Timer1 = 8.000.000 / 64 = 125.000 nhịp/giây
Số nhịp cho 1 ms  = 125
OCR1A            = 125 - 1 = 124
```

Bộ đếm chạy từ 0 đến 124, đủ 125 nhịp thì phát sinh ngắt so sánh. Trình phục vụ ngắt chỉ tăng đồng hồ:

```c
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
    ++system_ms;
}
```

`system_ms` là `volatile unsigned long`: nó có thể thay đổi do ngắt nên trình biên dịch phải đọc giá trị thực khi cần. ATmega128 là MCU 8 bit, còn đồng hồ có 32 bit; `timer_get_ms()` tạm khóa ngắt khi sao chép để không đọc lẫn các byte của hai thời điểm khác nhau.

Việc tìm vị trí, kiểm tra BFS và vẽ màn hình đều thực hiện trong vòng lặp chính. Chúng không chạy trong ngắt Timer1.

### 2.4. Tách thời gian đang chơi khỏi thời gian tạm dừng

Đầu mỗi vòng lặp chính:

```c
running_elapsed_ms=0;
if (app_state==APP_STATE_GAME && snake_state()==SNAKE_RUNNING) {
    running_elapsed_ms=(unsigned long)(now_ms-last_service_ms);
}
last_service_ms=now_ms;
```

Giải thích từng bước:

1. `now_ms` đã được lấy từ `timer_get_ms()`.
2. Đặt `running_elapsed_ms=0` để mặc định lượt này không có thời gian được tính cho gameplay.
3. Chỉ khi đang ở màn chơi và trạng thái là `SNAKE_RUNNING`, tính số ms từ lần phục vụ trước: `now_ms-last_service_ms`.
4. Luôn đưa `last_service_ms` về `now_ms`, kể cả khi đang Pause hoặc ở menu. Nhờ đó, khi Resume chương trình không cộng cả quãng tạm dừng vào chu kỳ.

Ví dụ đã tích lũy 7.000 ms rồi Pause: `elapsed_ms` giữ phần tích lũy đó. Dù đồng hồ MCU vẫn tăng trong lúc Pause, thời gian chờ không được cộng vào bộ sinh vật cản. Sau Resume, cần thêm khoảng 3.000 ms RUNNING để đến lượt tiếp theo. Mốc chuyển trạng thái được phục vụ theo vòng lặp nên có độ phân giải thực tế phụ thuộc thời gian xử lý vòng lặp.

Trong `main.c`, trình tự ở màn chơi là: xử lý phím → di chuyển rắn nếu đủ chu kỳ → thử cập nhật vật cản nếu rắn vẫn RUNNING. Vì vậy, nếu vừa thua hoặc thắng ở bước di chuyển thì không sinh thêm vật cản ở phần sau của lượt đó.

### 2.5. Kiểm tra đủ 10 giây và giới hạn 8 thanh

Phần đầu của `dynamic_obstacle_update()`:

```c
if (snake_state()!=SNAKE_RUNNING ||
    obstacle_count>=DYNAMIC_OBSTACLE_MAX_COUNT) return 0;
if (elapsed_running_ms>=DYNAMIC_OBSTACLE_INTERVAL_MS)
    elapsed_ms=DYNAMIC_OBSTACLE_INTERVAL_MS;
else {
    elapsed_ms+=elapsed_running_ms;
    if (elapsed_ms<DYNAMIC_OBSTACLE_INTERVAL_MS) return 0;
}
/* Drop excess elapsed time: never catch up with multiple spawns. */
elapsed_ms=0;
```

Hai tham số thời gian cần phân biệt:

- `elapsed_running_ms`: khoảng thời gian vừa trôi qua, được truyền vào ở một lần gọi.
- `elapsed_ms`: tổng tích lũy được lưu giữa các lần gọi, chờ đủ 10 giây.

Cách xử lý:

| Điều kiện | Kết quả |
|---|---|
| Không RUNNING hoặc đã có 8 thanh | Trả về `0`, không thay đổi danh sách vật cản |
| Khoảng thời gian mới từ 10.000 ms trở lên | Đưa tích lũy tới ngưỡng để thực hiện một lượt thử |
| Khoảng mới nhỏ hơn 10.000 ms | Cộng vào tổng; chưa đủ thì trả về `0` |
| Tổng đã đạt hoặc vượt ngưỡng | Đặt `elapsed_ms=0`, bắt đầu tìm vị trí |

Ví dụ `elapsed_ms=9.990` và lần gọi mới nhận 15 ms: tổng thành 10.005 ms, chương trình thử sinh một thanh rồi đặt tích lũy về 0. Phần dư 5 ms không được giữ lại.

Nếu một lần xử lý bị trễ nhiều giây, chương trình vẫn chỉ thử sinh tối đa một thanh trong lần gọi đó. Không có vòng lặp sinh bù nhiều thanh theo thời gian đã trễ. Các mốc 10/20/30 giây vì vậy là gần đúng theo thời gian RUNNING và lịch phục vụ, không phải lịch cứng theo đồng hồ ngoài đời.

### 2.6. Chọn điểm bắt đầu giả ngẫu nhiên và duyệt các vị trí

Đoạn tìm kiếm trong hàm cập nhật:

```c
offset=snake_random_index((unsigned char)(GRID_COLS*GRID_ROWS));
for (i=0;i<(unsigned char)(GRID_COLS*GRID_ROWS);++i) {
    unsigned char cell=(unsigned char)((offset+i)%(GRID_COLS*GRID_ROWS));
    x=(unsigned char)(cell%GRID_COLS);
    y=(unsigned char)(cell/GRID_COLS);
    if (dynamic_obstacle_candidate_valid(x,y)) {
        for (j=0;j<DYNAMIC_OBSTACLE_LENGTH;++j)
            blocked_rows[y]|=(unsigned int)(1u<<(x+j));
        obstacle_starts[obstacle_count].x=x;
        obstacle_starts[obstacle_count].y=y;
        ++obstacle_count;
        return 1;
    }
}
return 0;
```

`snake_random_index(112)` dùng bộ sinh giả ngẫu nhiên LFSR 16 bit của game rồi lấy phần dư cho 112. `offset` chỉ chọn điểm bắt đầu tìm kiếm; chương trình không bốc ngẫu nhiên lại sau mỗi vị trí bị loại.

Mỗi ô có một mã từ 0 đến 111:

```text
cell = y * 14 + x
x    = cell % 14
y    = cell / 14   (chia số nguyên)
```

Ví dụ chọn `offset=78`:

| `i` | `cell=(78+i)%112` | `(x,y)` được thử |
|---:|---:|---|
| 0 | 78 | `(8,5)` |
| 1 | 79 | `(9,5)` |
| 2 | 80 | `(10,5)` |
| 3 | 81 | `(11,5)` |
| 4 | 82 | `(12,5)` — bị loại vì thanh sẽ vượt cạnh phải |
| 5 | 83 | `(13,5)` — cũng vượt cạnh phải |
| 6 | 84 | `(0,6)` |

Dấu `%112` giúp sau ô 111 chương trình quay về ô 0. Vòng lặp tối đa 112 lượt nên mỗi tọa độ đầu thanh được xem xét một lần. Gặp vị trí hợp lệ đầu tiên thì ghi nhận và trả về, không duyệt tiếp để chọn vị trí tốt nhất.

Cách này làm vị trí bắt đầu thay đổi giữa các lượt, đồng thời bảo đảm tìm hết lưới nếu cần. Nó không bảo đảm mọi vị trí hợp lệ có xác suất được chọn bằng nhau.

### 2.7. Kiểm tra toàn bộ thanh có nằm trong lưới không

Hàm kiểm tra một vị trí đầu thanh:

```c
unsigned char dynamic_obstacle_candidate_valid(unsigned char x,unsigned char y)
{
    unsigned char j;
    if (x>GRID_COLS-DYNAMIC_OBSTACLE_LENGTH || y>=GRID_ROWS) return 0;
    for (j=0;j<DYNAMIC_OBSTACLE_LENGTH;++j)
        if (!cell_safe((unsigned char)(x+j),y)) return 0;
    return remains_connected(x,y);
}
```

Điều kiện đầu tiên là:

```c
if (x>GRID_COLS-DYNAMIC_OBSTACLE_LENGTH || y>=GRID_ROWS) return 0;
```

Với 14 cột và thanh dài 3 ô, đầu thanh phải thỏa `x<=11`, còn `y` từ 0 đến 7.

Ví dụ `(11,5)` chiếm cột 11, 12, 13 nên còn hợp lệ về biên. `(12,5)` cần cột 14 nên bị loại. Rắn có thể xuyên biên, nhưng hình dạng thanh chắn luôn phải nằm trọn trong màn hình.

Sau đó, vòng `for` lần lượt gọi `cell_safe()` với `(x,y)`, `(x+1,y)` và `(x+2,y)`. Chỉ cần một ô không an toàn thì trả về `0`; chưa ghi bất kỳ bit vật cản nào.

### 2.8. Kiểm tra từng ô bằng `cell_safe()`

Toàn bộ hàm:

```c
static unsigned char cell_safe(unsigned char x,unsigned char y)
{
    unsigned char i,distance,dx,dy;
    SnakePoint head,point,food;
    signed int next_x,next_y;
    if (x>=GRID_COLS || y>=GRID_ROWS ||
        game_map_is_blocked(x,y) || dynamic_obstacle_is_blocked(x,y)) return 0;
    head=snake_segment(0);
    dx=(unsigned char)((x>head.x)?(x-head.x):(head.x-x));
    dy=(unsigned char)((y>head.y)?(y-head.y):(head.y-y));
    if (snake_can_wrap()) {
        if (dx>GRID_COLS-dx) dx=(unsigned char)(GRID_COLS-dx);
        if (dy>GRID_ROWS-dy) dy=(unsigned char)(GRID_ROWS-dy);
    }
    distance=(unsigned char)(dx+dy);
    if (distance<=DYNAMIC_OBSTACLE_SAFE_DIST) return 0;
    snake_next_head(&next_x,&next_y);
    if (next_x==(signed int)x && next_y==(signed int)y) return 0;
    food=snake_food();
    if (x==food.x && y==food.y) return 0;
    for (i=0;i<snake_length();++i) {
        point=snake_segment(i);
        if (point.x==x && point.y==y) return 0;
    }
    return 1;
}
```

Hàm trả về `1` khi ô an toàn, `0` khi không được phép đặt. Các điều kiện được kiểm tra theo thứ tự dưới đây.

#### a. Loại ô ngoài lưới, có tường hoặc có vật cản cũ

Điều kiện đầu hàm loại các tọa độ vượt lưới, tường cố định và vật cản sinh thêm. Nhờ đó, các thanh không chồng lên nhau và không chồng lên tường bản đồ.

#### b. Tính khoảng cách đến đầu rắn, có xét xuyên biên

Trước tiên tính độ lệch cột `dx=|x-head.x|` và độ lệch hàng `dy=|y-head.y|`. Vì game cho xuyên biên, chương trình có thể chọn đường ngắn hơn qua cạnh:

```text
Khoảng ngang = min(dx, 14-dx)
Khoảng dọc   = min(dy, 8-dy)
Khoảng cách  = khoảng ngang + khoảng dọc
```

Đây là khoảng cách theo số bước ngang/dọc trên lưới xuyên biên, không phải độ dài đường đi thực tế tránh tường. Ví dụ:

| Đầu rắn | Ô cần xét | Khoảng cách | Kết quả theo vùng an toàn |
|---|---|---:|---|
| `(4,2)` | `(6,2)` | `2+0=2` | Loại |
| `(4,2)` | `(6,3)` | `2+1=3` | Qua bước khoảng cách |
| `(13,4)` | `(0,4)` | `1+0=1` qua cạnh | Loại |
| `(4,0)` | `(4,7)` | `0+1=1` qua cạnh | Loại |

`distance<=2` bị loại; từ 3 trở lên mới tiếp tục các bước kiểm tra khác. Điều kiện phải đúng với **cả ba ô** của thanh. Đầu trái xa đầu rắn không có nghĩa là hai ô còn lại cũng an toàn.

#### c. Loại ô đầu rắn dự kiến đi tới

`snake_next_head()` tính ô đầu rắn sẽ tới ở bước kế tiếp. Nó dùng `next_direction` nếu có lệnh rẽ đang chờ, nếu không thì dùng `direction`, rồi áp dụng xuyên biên.

Nếu tọa độ đó trùng ô đang thử thì loại. Với cấu hình khoảng cách an toàn hiện tại, ô đi tới trực tiếp thường đã nằm trong vùng bị loại ở bước trước; mã vẫn giữ kiểm tra rõ ràng cho ô dự kiến.

#### d. Loại ô mồi và mọi đốt rắn

Hàm lấy `snake_food()` để không đặt thanh đè lên mồi. Sau đó duyệt từ đốt 0 đến `snake_length()-1` và so sánh tọa độ. Nếu trùng đầu, thân hoặc đuôi đều bị loại.

Nếu vượt qua tất cả điều kiện, `cell_safe()` trả về `1`. Đây mới là an toàn của **một ô**; thanh vẫn phải qua kiểm tra liên thông sau khi đủ cả ba ô.

### 2.9. Giả lập thanh mới mà chưa sửa bản đồ thật

Một thanh có ba ô riêng lẻ an toàn vẫn có thể bít lối giữa hai vùng. Vì vậy, `dynamic_obstacle_candidate_valid()` chỉ chấp nhận sau khi `remains_connected(x,y)` thành công.

Bên trong BFS, hàm `is_wall()` quyết định một ô có bị chặn trong bản đồ giả lập hay không:

```c
static unsigned char is_wall(unsigned char x,unsigned char y,
                             unsigned char candidate_x,unsigned char candidate_y)
{
    return (unsigned char)(game_map_is_blocked(x,y) ||
        dynamic_obstacle_is_blocked(x,y) ||
        (y==candidate_y && x>=candidate_x &&
         x<candidate_x+DYNAMIC_OBSTACLE_LENGTH));
}
```

Ba nguyên nhân khiến một ô bị xem là tường:

1. Nó là tường cố định của map.
2. Nó nằm trong một thanh sinh thêm trước đó.
3. Nó cùng hàng với thanh đang thử và có cột từ `candidate_x` đến `candidate_x+2`.

Điểm quan trọng là chương trình chưa bật ba bit trong `blocked_rows[]`. Hàm chỉ thêm điều kiện khi kiểm tra. Nếu ứng viên không hợp lệ, có thể bỏ qua ngay mà không phải khôi phục bản đồ.

### 2.10. BFS dùng hàng đợi để kiểm tra liên thông như thế nào?

BFS là duyệt theo chiều rộng: bắt đầu từ đầu rắn, tìm các ô kề có thể tới, rồi tiếp tục từ các ô vừa tìm được cho đến khi không còn ô chờ.

Các biến trong `remains_connected()`:

| Biến | Ý nghĩa |
|---|---|
| `start` | Vị trí đọc tiếp theo trong `queue[]` |
| `end` | Vị trí ghi tiếp theo trong `queue[]` |
| `cell` | Mã ô vừa lấy khỏi hàng đợi |
| `x`, `y` | Tọa độ đổi ra từ mã ô |
| `seen` | Số ô đã lấy ra và xử lý |
| `free_count` | Tổng số ô không bị tường/vật cản chặn sau khi giả lập thanh mới |
| `visited[]` | Các ô đã được phát hiện, kể cả ô còn chờ trong hàng đợi |
| `wrap` | Có xét kết nối qua cạnh màn hình hay không |

#### a. Xóa dấu cũ và đếm số ô cần đi tới

Đầu mỗi lần kiểm tra, hàm đặt tất cả `visited[y]=0`. Sau đó duyệt 112 ô; ô nào `is_wall()` trả về 0 thì cộng vào `free_count`.

`free_count` bao gồm cả ô đang có thân rắn và mồi. Ở bước kiểm tra cấu trúc bản đồ, chỉ tường và vật cản được xem là chặn cố định; thân rắn sẽ còn di chuyển.

Ví dụ CLASSIC đã có hai thanh, tức 6 ô vật cản: thử thêm một thanh không chồng nhau thì `free_count=112-6-3=103`.

#### b. Đánh dấu và đưa một ô vào hàng đợi

Hàm trợ giúp `visit()`:

```c
static void visit(unsigned char x,unsigned char y,unsigned char *end)
{
    unsigned int mask=(unsigned int)(1u<<x);
    if (visited[y]&mask) return;
    visited[y]|=mask;
    queue[*end]=(unsigned char)(y*GRID_COLS+x);
    ++*end;
}
```

Ví dụ ô `(8,5)` có `mask=1u<<8`, tức bit 8. Nếu bit đó đã có trong `visited[5]`, hàm trả về ngay vì ô đã được phát hiện.

Nếu chưa có, hàm thực hiện:

1. Bật bit đã thăm trong `visited[y]`.
2. Đổi tọa độ sang mã `y*14+x` và ghi vào `queue[*end]`.
3. Tăng vị trí ghi `end` thông qua con trỏ `*end`.

Phải đánh dấu ngay khi đưa vào hàng đợi, thay vì đợi đến lúc lấy ra, để một ô không được đưa vào nhiều lần từ các ô hàng xóm khác nhau.

Sau khi đếm ô, BFS gọi `visit(head.x,head.y,&end)` để đưa đầu rắn vào hàng đợi. Điều kiện an toàn trước đó đã bảo đảm thanh đang thử không chiếm ô đầu rắn.

#### c. Lấy ô ra và xét các ô kề

Vòng `while (start<end)` tiếp tục khi hàng đợi còn ô chưa xử lý. Mỗi lượt lấy:

```c
cell=queue[start++];
x=(unsigned char)(cell%GRID_COLS);
y=(unsigned char)(cell/GRID_COLS);
++seen;
```

Sau đó thử trái, phải, trên, dưới. Mỗi ô hàng xóm chỉ được chuyển cho `visit()` khi nằm trong lưới và không bị `is_wall()` chặn.

Ví dụ lấy `(4,2)` thì các ô kề thông thường là `(3,2)`, `(5,2)`, `(4,1)`, `(4,3)`. Ô đã thăm sẽ tự bị `visit()` bỏ qua; ô mới được đưa xuống cuối hàng đợi để xử lý sau.

Nếu một ô nằm trên cạnh và `wrap=1`, hàm còn xét ô ở cạnh đối diện. Ví dụ `(0,2)` có thể nối sang `(13,2)`; `(4,0)` có thể nối sang `(4,7)`. Vì vậy, một thanh bịt đường trong vùng giữa màn hình chưa chắc chia cắt bản đồ nếu vẫn còn đường qua cạnh.

#### d. So sánh số ô đi tới được với tổng số ô cần đi tới

Sau khi hết hàng đợi:

```c
return (unsigned char)(seen==free_count &&
    (visited[food.y]&(1u<<food.x))!=0u);
```

Hai điều kiện đều phải đúng:

- `seen==free_count`: từ đầu rắn đã đi tới được mọi ô không bị chặn, nên bản đồ không bị chia thành các vùng tách rời.
- Bit của mồi có trong `visited[]`: ô mồi vẫn thuộc vùng đi tới được.

Với ví dụ `free_count=103`, nếu BFS chỉ đi tới 80 ô thì còn 23 ô bị tách khỏi đầu rắn: ứng viên bị loại. Nếu đi tới đủ 103 ô và mồi đã được đánh dấu, ứng viên qua kiểm tra.

Việc chỉ giữ đường từ đầu tới mồi là chưa đủ cho yêu cầu của code: dù mồi còn tới được, một vùng khác bị cô lập vẫn khiến `seen<free_count` và thanh bị loại.

Toàn bộ hàm để đối chiếu với các bước trên:

```c
static unsigned char remains_connected(unsigned char candidate_x,
                                        unsigned char candidate_y)
{
    unsigned char start=0,end=0,cell,x,y,seen=0,free_count=0,wrap;
    SnakePoint head;
    SnakePoint food;

    head = snake_segment(0);
    food = snake_food();
    wrap=snake_can_wrap();
    for (y=0;y<GRID_ROWS;++y) {
        visited[y]=0;
        for (x=0;x<GRID_COLS;++x)
            if (!is_wall(x,y,candidate_x,candidate_y)) ++free_count;
    }
    visit(head.x,head.y,&end);
    while (start<end) {
        cell=queue[start++];
        x=(unsigned char)(cell%GRID_COLS);
        y=(unsigned char)(cell/GRID_COLS);
        ++seen;
        if (x>0 && !is_wall((unsigned char)(x-1u),y,candidate_x,candidate_y))
            visit((unsigned char)(x-1u),y,&end);
        if (x+1u<GRID_COLS && !is_wall((unsigned char)(x+1u),y,candidate_x,candidate_y))
            visit((unsigned char)(x+1u),y,&end);
        if (y>0 && !is_wall(x,(unsigned char)(y-1u),candidate_x,candidate_y))
            visit(x,(unsigned char)(y-1u),&end);
        if (y+1u<GRID_ROWS && !is_wall(x,(unsigned char)(y+1u),candidate_x,candidate_y))
            visit(x,(unsigned char)(y+1u),&end);
        if (wrap) {
            if (x==0u && !is_wall((unsigned char)(GRID_COLS-1u),y,candidate_x,candidate_y))
                visit((unsigned char)(GRID_COLS-1u),y,&end);
            if (x==GRID_COLS-1u && !is_wall(0u,y,candidate_x,candidate_y))
                visit(0u,y,&end);
            if (y==0u && !is_wall(x,(unsigned char)(GRID_ROWS-1u),candidate_x,candidate_y))
                visit(x,(unsigned char)(GRID_ROWS-1u),&end);
            if (y==GRID_ROWS-1u && !is_wall(x,0u,candidate_x,candidate_y))
                visit(x,0u,&end);
        }
    }
    return (unsigned char)(seen==free_count &&
        (visited[food.y]&(1u<<food.x))!=0u);
}
```

BFS này bảo đảm tính liên thông theo tường/vật cản của bản đồ giả lập. Nó không mô phỏng mọi bước di chuyển tương lai của thân rắn và không bảo đảm người chơi sẽ tránh được mọi tình huống thua.

### 2.11. Ghi thanh hợp lệ vào dữ liệu thật

Chỉ sau khi kiểm tra từng ô và BFS đều thành công, hàm cập nhật mới ghi nhận:

```c
for (j=0;j<DYNAMIC_OBSTACLE_LENGTH;++j)
    blocked_rows[y]|=(unsigned int)(1u<<(x+j));
obstacle_starts[obstacle_count].x=x;
obstacle_starts[obstacle_count].y=y;
++obstacle_count;
return 1;
```

Phép `1u<<(x+j)` tạo một bit tương ứng với cột cần chặn. Toán tử `|=` bật bit đó và giữ nguyên các bit khác của hàng.

Với thanh bắt đầu tại `(8,5)`:

```text
j=0: bật bit 8  → 0x0100
j=1: bật bit 9  → 0x0200
j=2: bật bit 10 → 0x0400

Ba bit mới gộp lại: 0x0700
blocked_rows[5] |= 0x0700
```

Nếu hàng đã có thanh khác, các bit của thanh cũ vẫn được giữ lại. Kiểm tra an toàn đã bảo đảm ba bit mới không trùng vật cản cũ.

`obstacle_starts[obstacle_count]` lưu `(x,y)` vào ô danh sách còn trống. Sau đó `++obstacle_count` tăng số thanh đã đặt. Với 3 thanh, số ô bị chiếm là 9; biến đếm vẫn bằng 3.

Trả về `1` ngay sau khi ghi giúp kết thúc lượt sinh: không thể đặt thanh thứ hai trong cùng lần gọi.

### 2.12. Cập nhật Nokia 5110 và LCD

Bên gọi trong `main.c`:

```c
if (app_state==APP_STATE_GAME && snake_state()==SNAKE_RUNNING &&
    dynamic_obstacle_update(running_elapsed_ms)) {
    redraw=1;
    update_lcd=1;
}
```

`redraw=1` yêu cầu vẽ lại vùng chơi, còn `update_lcd=1` yêu cầu cập nhật nội dung LCD. Nếu hàm trả về `0` thì không có vật cản mới cần hiển thị.

Trong `game_render()`, chương trình lấy điểm đầu mỗi thanh qua `dynamic_obstacle_point(i)` và vẽ một hình chữ nhật tô đặc:

```c
for (i=0;i<dynamic_obstacle_get_count();++i) {
    point=dynamic_obstacle_point(i);
    display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                      (unsigned char)(point.y*CELL_SIZE),
                      DYNAMIC_OBSTACLE_LENGTH*CELL_SIZE,CELL_SIZE);
}
```

Mỗi ô có kích thước `CELL_SIZE=6` pixel. Thanh 3 ô vì vậy có kích thước `18×6` pixel. Tọa độ lưới `(8,5)` được đổi thành góc ảnh `(48,30)` pixel.

LCD 16×2 lấy `dynamic_obstacle_get_count()` để hiện số thanh trong chuỗi `OBST:xx`, cùng trạng thái PLAYING hoặc PAUSED. Số hiển thị là số thanh, không phải số ô.

### 2.13. Kiểm tra va chạm và cập nhật điều kiện thắng

Hàm tra cứu một ô vật cản:

```c
unsigned char dynamic_obstacle_is_blocked(signed int x,signed int y)
{
    if (x<0 || x>=GRID_COLS || y<0 || y>=GRID_ROWS) return 1;
    return (unsigned char)((blocked_rows[(unsigned char)y]>>x)&1u);
}
```

Sau khi kiểm tra biên, biểu thức dịch hàng sang phải `x` bit rồi `&1u` lấy đúng bit của ô `(x,y)`. Kết quả 1 nghĩa là ô đang bị chặn.

Trong `snake_step()`, đầu rắn dự kiến được xử lý xuyên biên rồi mới kiểm tra:

```c
if (game_map_is_blocked(new_x,new_y) ||
    dynamic_obstacle_is_blocked(new_x,new_y)) {
    state=SNAKE_GAME_OVER;
    return 1;
}
```

Nếu va chạm, hàm trả về trước khi dịch mảng rắn. Đầu không được chuyển vào trong thanh chắn.

Số ô cần chiếm để thắng cũng được tính lại bằng `dynamic_obstacle_playable_cells()`: lấy số ô không có tường cố định trừ số ô vật cản sinh thêm. Ví dụ CLASSIC có 2 thanh thì còn `112-6=106` ô chơi hợp lệ.

### 2.14. Không tìm được vị trí thì xử lý ra sao?

Nếu cả 112 điểm đầu thanh đều bị loại, vòng tìm kiếm kết thúc và `dynamic_obstacle_update()` trả về `0`. Không có thay đổi trong `blocked_rows`, `obstacle_starts` hoặc `obstacle_count`.

Tuy nhiên, `elapsed_ms` đã được đặt lại về 0 trước khi tìm kiếm. Chương trình sẽ chờ thêm khoảng 10 giây RUNNING mới thử lại, không tìm lại liên tục ở mọi vòng lặp.

Các nguyên nhân bị loại có thể gồm: vượt cạnh, trùng rắn/mồi/tường/thanh cũ, quá gần đầu, trùng ô đầu dự kiến hoặc làm vùng chơi mất liên thông.

### 2.15. Pause, Restart và quan hệ với đảo đầu–đuôi

Khi Pause, phần thời gian tích lũy được giữ lại và chương trình không gọi cập nhật vật cản trong trạng thái PAUSED. Bản đồ và các thanh đã sinh vẫn giữ nguyên.

Khi bắt đầu ván hoặc nhấn phím 9, `snake_init()` gọi:

```c
void dynamic_obstacle_reset(void)
{
    unsigned char y;
    for (y=0;y<GRID_ROWS;++y) blocked_rows[y]=0;
    obstacle_count=0;
    elapsed_ms=0;
}
```

Sau reset, mọi bit vật cản sinh thêm bằng 0, số thanh bằng 0 và thời gian tích lũy bằng 0. Mảng `obstacle_starts[]` không cần xóa từng phần tử vì `obstacle_count=0` khiến chương trình không đọc các phần tử cũ để vẽ. Khi sinh thanh mới, dữ liệu sẽ được ghi đè từ chỉ số 0.

Trong `main.c`, nhánh Restart còn đặt `running_elapsed_ms=0` và cập nhật `last_service_ms`, tránh tính quãng thời gian của ván cũ vào ván mới. Tường cố định vẫn được giữ nguyên vì chúng nằm trong dữ liệu map riêng.

Trong HARD, đảo đầu–đuôi khi ăn không gọi `dynamic_obstacle_reset()`. Vật cản cũ và đồng hồ vẫn tồn tại. Do `snake_step()` chạy trước bước cập nhật vật cản, nếu cùng một lượt vừa ăn vừa đến chu kỳ sinh, các phép kiểm tra an toàn sẽ đọc đầu, thân và mồi sau khi bước ăn/đảo đã hoàn tất.

### 2.16. Giới hạn xử lý của thuật toán

Lưới chỉ có 112 ô. Hàng đợi BFS chứa tối đa 112 ô khác nhau vì `visit()` đánh dấu trước khi thêm; chỉ số `unsigned char` đủ chứa các giá trị từ 0 đến 112 trong cấu hình hiện tại.

Mỗi lượt thử duyệt tối đa 112 điểm đầu thanh. Với mỗi điểm, chương trình kiểm tra 3 ô và chỉ chạy BFS khi ba ô đã an toàn. Một BFS duyệt tối đa 112 ô, mỗi ô xét tối đa bốn kết nối theo luật của lưới xuyên biên.

Các con số này mô tả giới hạn số bước của cấu hình hiện tại, không phải thời gian chạy đã đo trên MCU. Việc dùng mảng cố định, không cấp phát động và thực hiện tìm kiếm trong vòng lặp chính giúp quản lý bộ nhớ và luồng xử lý rõ ràng.
