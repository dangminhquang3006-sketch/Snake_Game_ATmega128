# Bản đồ và tường cố định

## 1. Ví dụ dễ hình dung

Game có lưới 14 cột × 8 hàng. CLASSIC không có tường cố định. PILLARS có các cụm cột; LABYRINTH có các vách tạo hành lang.

Ví dụ PILLARS, ký hiệu `#` là tường, `.` là ô chơi:

```text
Cột:   01234567890123
Hàng 0 ..............
Hàng 1 ..............
Hàng 2 ....##..##....
Hàng 3 ..............
Hàng 4 ..............
Hàng 5 ....##..##....
Hàng 6 ..............
Hàng 7 ..............
```

Có 8 ô tường nên còn `112-8=104` ô chơi trước khi sinh thêm vật cản. Đầu ở `(3,2)` đi RIGHT sẽ gặp tường `(4,2)` và GAME OVER.

| Map | Số ô tường cố định | Số ô chơi ban đầu |
|---|---:|---:|
| CLASSIC | 0 | 112 |
| PILLARS | 8 | 104 |
| LABYRINTH | 12 | 100 |

Map được người chơi chọn trước ván và giữ nguyên đến khi chọn lại. Tường có sẵn không bị xóa khi nhấn 9. Các thanh chắn sinh theo thời gian là một lớp dữ liệu riêng, xem [SINH_VAT_CAN.md](SINH_VAT_CAN.md).

## 2. Cách chương trình thực hiện

Dữ liệu và kiểm tra map nằm trong [game/game.c](../game/game.c); các ID và API nằm trong [game/game_map.h](../game/game_map.h).

### 2.1. Gán ID và lưu 8 hàng của từng map

```c
typedef enum {
    MAP_CLASSIC=0,
    MAP_PILLARS,
    MAP_LABYRINTH,
    MAP_COUNT
} MapId;
```

```c
static flash unsigned int walls[MAP_COUNT][GRID_ROWS]={
    {0,0,0,0,0,0,0,0},
    {0,0,0x0330u,0,0,0x0330u,0,0},
    {0,0x0408u,0x04c8u,0,0,0x0cccu,0,0}
};
static MapId current_map=MAP_CLASSIC;
```

`walls[map][y]` lưu một hàng của một map. `flash` đặt bảng tường trong bộ nhớ chương trình theo cú pháp CodeVisionAVR; dữ liệu tường không cần sửa khi đang chơi.

Trong CodeVision cho ATmega128, một `unsigned int` có 16 bit. Mỗi hàng chỉ có 14 cột nên dùng 14 bit thấp. Ba map × 8 hàng × 2 byte tạo bảng dữ liệu tường 48 byte.

`current_map` là map đang có hiệu lực trong nhóm bản đồ. Con trỏ `selected_map` của menu được giữ riêng; chọn bằng phím lên/xuống chưa áp dụng map ngay.

### 2.2. Một bit tương ứng với một cột

```c
static unsigned char blocked_at(MapId map, unsigned char x, unsigned char y)
{
    return (unsigned char)((walls[(unsigned char)map][y] >> x)&1u);
}
```

`>>x` đưa bit cột `x` về vị trí thấp nhất; `&1u` lấy giá trị 0 hoặc 1 của bit đó. Bit 1 là tường, bit 0 là ô không có tường cố định.

Ví dụ `0x0330` có các bit 4, 5, 8, 9 được bật:

```text
0x0010 + 0x0020 + 0x0100 + 0x0200 = 0x0330
    x=4      x=5      x=8      x=9
```

Bảng PILLARS đặt giá trị này ở hàng 2 và 5, tạo ra 4 ô tường mỗi hàng. Cột trong sơ đồ tăng từ trái sang phải theo `x`; khi viết số nhị phân thông thường, bit thấp lại được in ở bên phải, nên không nên đọc vị trí cột chỉ bằng hình thức chuỗi nhị phân.

### 2.3. Đọc các vách của LABYRINTH

| Hàng | Mặt nạ | Các cột có tường |
|---:|---|---|
| 1 | `0x0408` | 3, 10 |
| 2 | `0x04c8` | 3, 6, 7, 10 |
| 5 | `0x0ccc` | 2, 3, 6, 7, 10, 11 |
| Các hàng khác | 0 | Không có tường cố định |

Tổng cộng `2+4+6=12` ô tường. Những ô còn lại là vùng rắn có thể đi qua nếu không có thân hoặc vật cản sinh thêm tại thời điểm xét.

### 2.4. Kiểm tra ID và các bit ngoài lưới

Trước khi áp dụng map, `game_map_validate()` tạo `valid_mask=(1u<<14)-1u`, tức `0x3FFF`, rồi kiểm tra ID có nhỏ hơn `MAP_COUNT` hay không.

Nếu một hàng có bit ngoài 14 cột hợp lệ, phép `walls[map][y] & ~valid_mask` phát hiện và loại map. Điều này ngăn dữ liệu vô tình khai báo tường tại cột 14 hoặc 15.

Toàn bộ hàm kiểm tra:

```c
unsigned char game_map_validate(MapId map)
{
    unsigned int visited[GRID_ROWS]={0,0,0,0,0,0,0,0};
    unsigned char x,y,count=0,reached=0,changed;
    unsigned int valid_mask=(unsigned int)((1u<<GRID_COLS)-1u);
    if ((unsigned int)map>=MAP_COUNT) return 0;
    for (y=0; y<GRID_ROWS; ++y) {
        if (walls[(unsigned char)map][y] & (unsigned int)~valid_mask) return 0;
        for (x=0; x<GRID_COLS; ++x)
            if (!blocked_at(map,x,y)) ++count;
    }
    if (count<SNAKE_MAP_MIN_CELLS || blocked_at(map,5,4) ||
        blocked_at(map,6,4) || blocked_at(map,7,4)) return 0;
    /* Fixed layouts also have an exact expected capacity. */
    if ((map==MAP_CLASSIC && count!=112u) ||
        (map==MAP_PILLARS && count!=104u) ||
        (map==MAP_LABYRINTH && count!=100u)) return 0;
    visited[4]=(unsigned int)(1u<<7);
    do {
        changed=0;
        for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x) {
            unsigned int mask=(unsigned int)(1u<<x);
            if (blocked_at(map,x,y) || (visited[y]&mask)) continue;
            if ((x>0 && (visited[y]&(mask>>1))) ||
                (x+1<GRID_COLS && (visited[y]&(mask<<1))) ||
                (y>0 && (visited[y-1]&mask)) ||
                (y+1<GRID_ROWS && (visited[y+1]&mask))) {
                visited[y]|=mask;
                changed=1;
            }
        }
    } while (changed);
    for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x)
        if (visited[y] & (1u<<x)) ++reached;
    return (unsigned char)(reached==count);
}
```

Các phép kiểm tra đầu hàm xảy ra trước việc áp dụng map vào `current_map`; nếu thất bại thì lựa chọn không trở thành bản đồ đang chơi.

### 2.5. Đếm ô chơi và bảo đảm nơi xuất phát không có tường

Hàm duyệt 112 ô, đếm những ô `blocked_at()` trả về 0 vào biến `count`. Nó yêu cầu ít nhất `SNAKE_MAP_MIN_CELLS=4` và ba ô `(5,4)`, `(6,4)`, `(7,4)` phải trống.

Ba tọa độ này chính là đuôi, thân và đầu lúc `snake_init()` tạo rắn 3 đốt. Kiểm tra giúp rắn không xuất hiện ngay trong tường.

Ngoài giới hạn tối thiểu, hàm còn yêu cầu chính xác 112/104/100 ô theo từng ID map. Do đó, thay mặt nạ tường làm đổi số ô chơi mà không sửa điều kiện tương ứng sẽ khiến map bị từ chối, dù hình vẽ mới nhìn có vẻ hợp lý.

### 2.6. Kiểm tra vùng chơi liên thông bằng lan dấu lặp

Phần này của `game_map_validate()` không dùng hàng đợi BFS như bộ sinh vật cản. Nó dùng mảng mặt nạ `visited[8]` và quét lặp đến khi không đánh dấu được ô mới.

| Biến | Vai trò |
|---|---|
| `visited[y]` | Các ô đã xác định đi tới được trên hàng `y` |
| `changed` | Trong lượt quét này có thêm ô mới hay không |
| `count` | Tổng số ô không phải tường |
| `reached` | Tổng số ô được đánh dấu sau khi lan xong |

Trình tự:

1. Đánh dấu đầu xuất phát `(7,4)` bằng `visited[4]=1u<<7`.
2. Quét các ô theo hàng/cột; bỏ qua ô tường và ô đã đánh dấu.
3. Với ô chưa đánh dấu, kiểm tra có ô kề trái/phải/trên/dưới đã được đánh dấu hay không.
4. Nếu có, bật bit của ô đó và đặt `changed=1`.
5. Quét lại khi còn `changed`; dừng khi không tìm được ô mới.
6. Đếm `reached` và yêu cầu `reached==count`.

Một lượt quét có thể lan qua nhiều ô, vì dấu mới được dùng ngay trong lượt đó. Thuật toán vẫn cần quét lặp để lan theo những hướng ngược chiều thứ tự quét.

Kiểm tra map cố định chỉ dùng hàng xóm trong lưới, **không dùng nối qua cạnh**. Các map hiện tại phải liên thông ngay cả khi bỏ kết nối xuyên biên. Bộ kiểm tra thanh chắn động lại xét xuyên biên, đúng theo luật di chuyển khi chơi; đây là hai hàm khác nhau.

### 2.7. Chỉ áp dụng map sau khi kiểm tra thành công

```c
unsigned char game_map_select(MapId map)
{
    if (!game_map_validate(map)) return 0;
    current_map=map;
    return 1;
}
```

Nếu `game_map_validate()` trả về 0, `current_map` giữ nguyên. Nếu trả về 1, hàm ghi ID mới và báo thành công cho `main.c` bắt đầu ván.

Không có cấp phát lại lưới hay sao chép toàn bộ bảng tường khi chọn map. Chương trình chỉ thay ID để các phép đọc tiếp theo lấy đúng hàng trong bảng `walls`.

### 2.8. Dùng chung dữ liệu map cho va chạm và sinh mồi

```c
unsigned char game_map_is_blocked(signed int x, signed int y)
{
    if (x<0 || x>=GRID_COLS || y<0 || y>=GRID_ROWS) return 1;
    return blocked_at(current_map,(unsigned char)x,(unsigned char)y);
}
```

Hàm kiểm tra tọa độ trước khi gọi `blocked_at()`. Tọa độ ngoài lưới được xem là bị chặn, tránh truy cập sai mảng. Luồng di chuyển xử lý xuyên biên trước khi gọi hàm này.

Cùng một API được dùng khi:

- Đầu rắn chuẩn bị đi vào ô mới: có tường thì GAME OVER.
- Sinh mồi: không đặt vào tường.
- Sinh thanh chắn: không đặt đè lên tường; BFS tính cả tường trong bản đồ giả lập.
- Vẽ bản đồ: ô có tường được vẽ khung.
- Tính sức chứa: đếm các ô không có tường cho điều kiện WIN.

Nhờ dùng chung hàm, hình vẽ và luật va chạm đều dựa trên cùng dữ liệu bit của map.

### 2.9. Số ô chơi thay đổi khi có thanh chắn

`game_map_free_cells_for(map)` đếm ô không có tường cố định của một ID; `game_map_get_free_cells()` lấy số đó cho map hiện tại. Chúng chưa trừ thanh chắn sinh thêm.

`dynamic_obstacle_playable_cells()` mới trừ lớp vật cản động. Ví dụ PILLARS có hai thanh thì còn `104-6=98` ô chơi hợp lệ. Khi Restart, thanh bị xóa và số ô trở về 104, còn 8 ô tường ban đầu vẫn tồn tại.

### 2.10. Hiển thị tường khác thanh chắn

Trong `game_render()`, chương trình vẽ map bằng `display_rect()` cho mỗi ô tường, kích thước `6×6` pixel. Thanh sinh thêm được vẽ tô đặc bằng `display_fill_rect()`, kích thước `18×6` pixel cho cả ba ô.

Ví dụ tường `(4,2)` có góc trái trên tại `(24,12)` pixel. Dữ liệu logic vẫn lưu theo ô; chỉ khi vẽ mới nhân với `CELL_SIZE=6` để đổi sang pixel.
