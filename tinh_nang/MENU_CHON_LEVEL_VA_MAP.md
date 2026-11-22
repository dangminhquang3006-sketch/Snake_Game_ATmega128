# Menu chọn Level, Map và chuyển màn hình

## 1. Ví dụ dễ hình dung

Khi khởi động, Nokia hiển thị hai lựa chọn NORMAL/HARD, mặc định con trỏ ở NORMAL. Muốn chơi HARD trên map PILLARS:

| Thao tác | Màn hình và kết quả |
|---|---|
| Khởi động | Menu Level, con trỏ `> NORMAL` |
| Nhấn 8 rồi nhả | Con trỏ chuyển xuống `> HARD` |
| Nhấn 5 rồi nhả | Xác nhận HARD, hiện menu Map |
| Nhấn 8 rồi nhả | Từ CLASSIC xuống PILLARS, nếu con trỏ Map đang ở mặc định |
| Nhấn 5 rồi nhả | Kiểm tra map rồi bắt đầu ván HARD–PILLARS |
| Khi GAME OVER/WIN, nhấn 1 | Quay về menu Level |

```text
Menu Level ──5──> Menu Map ──5, map hợp lệ──> Chơi
    ^                │                       │
    └──────1─────────┘                       v
    └────────────1──────────────────── GAME OVER/WIN
```

Đây là chuyển giữa các màn hình của ứng dụng. Trong một ván, map không tự đổi sang map khác khi đạt điểm hoặc ăn mồi. Phím 1 ở màn Map quay về Level; trong ván đang chạy, phím 1 không đưa về menu. Phím 9 chơi lại ngay trên Level và Map hiện tại.

## 2. Cách chương trình thực hiện

Menu được triển khai trong [game/game.c](../game/game.c); [main.c](../main.c) giữ trạng thái màn hình và thực hiện việc bắt đầu ván. Bản đồ được giải thích riêng trong [BAN_DO_VA_TUONG_CO_DINH.md](BAN_DO_VA_TUONG_CO_DINH.md).

### 2.1. Tách trạng thái ứng dụng khỏi trạng thái rắn

```c
typedef enum { APP_STATE_LEVEL_SELECT=0, APP_STATE_MAP_SELECT,
               APP_STATE_GAME } AppState;
```

| Trạng thái ứng dụng | Nội dung xử lý |
|---|---|
| `APP_STATE_LEVEL_SELECT` | Nhận lựa chọn NORMAL/HARD |
| `APP_STATE_MAP_SELECT` | Nhận lựa chọn CLASSIC/PILLARS/LABYRINTH |
| `APP_STATE_GAME` | Chạy ván hoặc hiện kết quả của ván |

`SnakeState` là biến trạng thái khác: RUNNING/GAME OVER/WIN. GAME OVER không phải trạng thái thứ tư của `AppState`; màn kết quả vẫn thuộc `APP_STATE_GAME`.

Từ khóa `typedef enum` đặt tên cho các giá trị trạng thái để các nhánh `if` đọc được theo ý nghĩa, thay vì so sánh những con số khó nhớ.

### 2.2. Phân biệt con trỏ lựa chọn với cấu hình đã xác nhận

| Dữ liệu | Nơi giữ | Ý nghĩa |
|---|---|---|
| `selected_level` | `game.c` | Level đang được trỏ tới trong menu |
| `selected_map` | `game.c` | Map đang được trỏ tới trong menu |
| `current_level` | Biến cục bộ của `main()` | Level người dùng đã xác nhận |
| `current_map` | Biến cục bộ của `main()` | Map đã được chọn thành công cho ván |

Trong nhóm xử lý rắn cũng có một biến `current_level` riêng của `game.c`; nhóm bản đồ có `current_map` riêng. `snake_set_level()` và `game_map_select()` đưa cấu hình đã xác nhận vào các nhóm đó. Cùng tên biến không có nghĩa chúng là một vùng nhớ.

Di chuyển con trỏ chưa làm rắn bắt đầu chạy. Chương trình chỉ tạo ván sau khi xác nhận Map thành công.

### 2.3. Khởi tạo menu lúc bật chương trình

Ở nhánh game bình thường của `main()`, sau khi khởi tạo LCD, LED, keypad trước khi hiển thị menu:

```c
game_menu_init(GAME_LEVEL_NORMAL);
current_level=GAME_LEVEL_NORMAL;
current_map=MAP_CLASSIC;
game_map_menu_init(current_map);
move_period_ms=game_get_move_period(current_level);
app_state=APP_STATE_LEVEL_SELECT;
shown_score=0;
led7seg_set_number(0);
lcd1602_show_menu(current_level);
```

NORMAL và CLASSIC là các giá trị mặc định; `game_map_menu_init(current_map)` đặt con trỏ Map theo giá trị đó. `app_state` được đặt về màn Level. LED bằng 0, LCD hiển thị Level đang chọn.

Lúc này chưa gọi `snake_init()` cho một ván mới. Sau khi Timer và ngắt được bật, chương trình vẽ menu rồi đi vào vòng lặp phục vụ phím.

### 2.4. Ánh xạ phím theo ngữ cảnh menu

Keypad đổi phím vật lý 2/8/5 thành `KEY_UP`, `KEY_DOWN`, `KEY_CONFIRM`. Phím 5 chỉ xác nhận lựa chọn trong menu; khi đang chơi nó không tạo hành động.

| Phím vật lý | Sự kiện | Level | Map |
|---|---|---|---|
| 2 | `KEY_UP` | Chọn lên | Chọn lên |
| 8 | `KEY_DOWN` | Chọn xuống | Chọn xuống |
| 5 | `KEY_CONFIRM` | Xác nhận Level | Xác nhận Map |
| 1 | `KEY_MENU` | Không có hành động menu đặc biệt | Quay về Level |

Mỗi lần nhấn cần được nhả trước lần tiếp theo vì keypad không tự lặp sự kiện khi giữ phím. Xem [DIEU_KHIEN_KEYPAD.md](DIEU_KHIEN_KEYPAD.md).

### 2.5. Thay đổi lựa chọn Level và quay vòng danh sách

```c
unsigned char game_menu_handle_key(KeyEvent key)
{
    if (key==KEY_UP) {
        if (selected_level==GAME_LEVEL_NORMAL) selected_level=GAME_LEVEL_HARD;
        else selected_level=(GameLevel)(selected_level-1);
    } else if (key==KEY_DOWN) {
        if (selected_level==GAME_LEVEL_HARD) selected_level=GAME_LEVEL_NORMAL;
        else selected_level=(GameLevel)(selected_level+1);
    } else if (key==KEY_CONFIRM) return 1;
    return 0;
}
```

`GAME_LEVEL_NORMAL=1`, `GAME_LEVEL_HARD=2`; đây là các ID Level của chương trình. Chương trình không đưa ID 0 thành một Level trong menu.

- Nhấn UP ở NORMAL thì quay vòng lên HARD.
- Nhấn DOWN ở HARD thì quay vòng về NORMAL.
- Với hai lựa chọn, nhấn UP hoặc DOWN đều đổi sang lựa chọn còn lại.
- Nhấn 5 làm hàm trả về 1, báo yêu cầu xác nhận.
- Các lần chỉ đổi con trỏ trả về 0; trạng thái menu vẫn được cập nhật qua `selected_level`.

`main.c` lưu `previous_selection` trước khi gọi. Nếu lựa chọn đổi thì bật cờ vẽ, còn nếu xác nhận thì chép Level vào `current_level` và chuyển `app_state` sang Map.

### 2.6. Thay đổi lựa chọn Map

```c
unsigned char game_map_menu_handle_key(KeyEvent key)
{
    if (key==KEY_UP) {
        if (selected_map==MAP_CLASSIC) selected_map=MAP_LABYRINTH;
        else selected_map=(MapId)(selected_map-1);
    } else if (key==KEY_DOWN) {
        if (selected_map==MAP_LABYRINTH) selected_map=MAP_CLASSIC;
        else selected_map=(MapId)(selected_map+1);
    } else if (key==KEY_CONFIRM) return 1;
    return 0;
}
```

Map có ID CLASSIC=0, PILLARS=1, LABYRINTH=2. DOWN đi `CLASSIC → PILLARS → LABYRINTH → CLASSIC`; UP đi theo chiều ngược lại.

`game_map_menu_selected()` chỉ trả về con trỏ menu, chưa làm thay đổi `current_map` trong bộ xử lý bản đồ. Muốn áp dụng map phải qua bước kiểm tra bằng `game_map_select()`.

Trong code hiện tại, con trỏ Map được khởi tạo ở đầu chương trình, không đặt lại về CLASSIC mỗi lần vừa xác nhận Level. Do đó, sau khi đã thao tác menu trước đó, con trỏ có thể còn ở map đã trỏ tới.

### 2.7. Xác nhận Map và tạo ván mới theo đúng thứ tự

Khi nhấn 5 trong menu Map và `game_map_select()` trả về thành công:

```c
current_map=game_map_get_current();
move_period_ms=game_get_move_period(current_level);
snake_set_level(current_level);
snake_init();
app_state=APP_STATE_GAME;
last_move_ms=now_ms;
last_service_ms=now_ms;
shown_score=0;
led7seg_set_number(0);
redraw=1;
update_lcd=1;
```

| Thao tác trong code | Mục đích |
|---|---|
| Lấy `game_map_get_current()` | Đồng bộ map đã áp dụng vào biến của `main()` |
| Lấy `game_get_move_period(current_level)` | NORMAL 300 ms/ô; HARD 250 ms/ô |
| `snake_set_level(current_level)` | Đưa luật Level vào nhóm xử lý rắn |
| `snake_init()` | Tạo rắn 3 đốt, điểm 0, mồi mới, xóa vật cản sinh thêm |
| Đặt `app_state=APP_STATE_GAME` | Chuyển vòng lặp sang xử lý ván |
| Đặt lại các mốc thời gian | Không tính thời gian đứng ở menu vào bước đi hoặc chu kỳ vật cản |
| Bật `redraw`, `update_lcd` | Hiện vùng chơi và thông tin ván mới |

Nếu map không hợp lệ, code hiện `INVALID MAP`; không gọi `snake_init()` và không chuyển sang trạng thái chơi. Không có bước chọn STANDARD/SURVIVAL riêng giữa Map và game.

### 2.8. Vẽ con trỏ và nội dung màn hình

```c
void game_menu_render(void)
{
    display_clear();
    display_text(0,0,title_text);
    if (selected_level==GAME_LEVEL_NORMAL) display_text(0,14,normal_selected);
    else display_text(0,14,normal_plain);
    if (selected_level==GAME_LEVEL_HARD) display_text(0,26,hard_selected);
    else display_text(0,26,hard_plain);
    display_text(0,40,help_text);
}
```

Menu Level dùng chuỗi có dấu `>` cho mục được chọn, chuỗi không có dấu cho mục còn lại. `display_clear()` xóa nội dung cũ trước khi vẽ toàn menu.

Menu Map làm tương tự với ba lựa chọn và hiện Level đã xác nhận ở phía trên. Việc đổi màn hình là đổi trạng thái rồi gọi hàm vẽ phù hợp, không phải tải một trò chơi khác.

Cuối vòng lặp, `redraw=1` khiến `main.c` chọn `game_menu_render()`, `game_map_menu_render()` hoặc `game_render()` theo `app_state`. Cờ `update_lcd` điều khiển nhánh chuẩn bị nội dung LCD tương ứng.

### 2.9. Hiển thị lựa chọn và quay lại bước trước

Ở menu Level, LCD hiện `SELECT LEVEL` và tên Level đang trỏ tới. Ở menu Map, LCD hiện `SELECT MAP` và tên Map đang trỏ tới. Phần hiển thị chỉ cần lựa chọn hiện tại, không đọc dữ liệu lưu trữ.

Phím 1 trong màn Map đổi `app_state` về Level và yêu cầu vẽ lại; không tạo một ván chạy nền. Sau WIN/GAME OVER, phím 1 cũng về Level và đưa LED về 0. Dữ liệu rắn được khởi tạo lại khi xác nhận Map để chơi tiếp.

Phím 9 ở màn kết quả giữ Level/Map rồi gọi `snake_init()` trực tiếp. Chi tiết nằm trong [CHET_VA_GAME_OVER.md](CHET_VA_GAME_OVER.md).
