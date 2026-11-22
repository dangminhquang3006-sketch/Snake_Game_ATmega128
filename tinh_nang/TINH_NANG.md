# Các tính năng của Snake Game ATmega128

Tài liệu mô tả bản **7 file mã nguồn gốc** và bản **gộp 2 file** trong `export/codevision/`. Bản gộp được sinh từ các source gốc bằng `export/generate_codevision.py`, dùng cùng logic và cấu hình tính năng; các giải thích bên dưới áp dụng cho cả hai cách đóng gói.

## 1. Hai chế độ chơi

| Chế độ | Tốc độ | Đặc điểm |
|---|---|---|
| NORMAL | 300 ms/ô | Rắn đi qua một cạnh màn hình sẽ xuất hiện ở cạnh đối diện |
| HARD | 250 ms/ô | Xuyên biên và đảo đầu–đuôi sau khi ăn mồi, nếu chưa thắng |

Mặc định chọn NORMAL. Luồng bắt đầu: **chọn NORMAL/HARD → chọn map → chơi**.

## 2. Ba bản đồ

| Map | Đặc điểm | Số ô chơi ban đầu |
|---|---|---:|
| CLASSIC | Không có tường cố định | 112 |
| PILLARS | Có các cụm cột chắn | 104 |
| LABYRINTH | Có vách tạo hành lang | 100 |

Có tổng cộng **6 tổ hợp** chế độ × bản đồ. Số ô trên được tính trước khi sinh rào chắn.

## 3. Rào chắn theo thời gian

- Cả NORMAL và HARD đều thử sinh thêm **một thanh chắn ngang dài 3 ô mỗi 10 giây đang chơi**.
- Tối đa **8 thanh chắn** trong một ván; các thanh đã sinh giữ nguyên vị trí đến khi chơi lại.
- Cả thanh phải nằm trong màn hình, tránh tường, thân rắn, mồi, vùng cách đầu rắn tối đa 2 bước và ô đầu rắn dự kiến đi tới.
- Chỉ đặt thanh nếu các ô chơi còn liên thông theo luật xuyên biên. Không có vị trí hợp lệ thì bỏ qua lượt sinh.
- Chơi lại xóa toàn bộ rào chắn và đặt lại lịch; chỉ tính thời gian ván RUNNING.

## 4. Ba loại mồi và tính điểm

| Loại mồi | Xác suất | Điểm |
|---|---:|---:|
| Hình tròn (CIRCLE) | 60% | +10 |
| Hình vuông (SQUARE) | 35% | +20 |
| Ngôi sao (STAR) | 5% | +50 |

Ăn mỗi loại mồi đều tăng chiều dài rắn một đốt. Vị trí và loại mồi được chọn giả ngẫu nhiên; mồi chỉ xuất hiện ở ô trống, không trùng rắn hoặc vật cản.

## 5. Di chuyển và điều khiển

Rắn bắt đầu với **3 đốt**, hướng sang phải, điểm bằng 0. Rắn tự tiến theo tốc độ chế độ đã chọn; phím điều hướng chỉ đổi hướng. Không cho quay ngược trực tiếp hoặc nhận nhiều lệnh rẽ trong cùng một bước di chuyển.

| Phím | Trong menu | Trong game |
|---|---|---|
| 2 / 8 | Chọn lên / xuống | Đi lên / xuống |
| 4 / 6 | — | Đi trái / phải |
| 5 | Xác nhận | Không có hành động |
| 9 | — | Chơi lại |
| 1 | Từ menu Map về menu chế độ | Về menu chế độ sau khi thắng hoặc thua |

Keypad có chống dội phím; giữ phím không tạo liên tiếp nhiều sự kiện nhấn.

## 6. Chơi lại, thắng và thua

- **Chơi lại:** giữ chế độ và map đã chọn, đưa rắn về 3 đốt, điểm về 0, xóa rào chắn và đặt lại thời gian.
- **Game Over:** đầu rắn đâm vào thân, tường cố định hoặc rào chắn.
- **Win:** rắn chiếm hết các ô chơi hợp lệ còn lại.
- Màn hình cuối ván hiển thị điểm và hướng dẫn chơi lại hoặc về menu.

## 7. Hiển thị trên ba thiết bị

| Thiết bị | Nội dung |
|---|---|
| Nokia 5110, 84×48 pixel | Menu, vùng chơi 14×8 ô, rắn, mồi, tường, rào chắn và kết quả |
| LED 7 đoạn, 4 chữ số | Điểm hiện tại; BT1 chọn CA, BT2 chọn CC, lưu mode trong EEPROM |
| LCD 16×2 | Chế độ, map, số rào chắn, trạng thái PLAYING, lựa chọn menu và kết quả cuối ván |

Mode được đọc lại từ EEPROM khi khởi động. CA dùng NOT cho mẫu segment và digit active-high; CC giữ mẫu segment và digit active-low. Lưu cấu hình hiển thị không lưu điểm hay kỷ lục.

## 8. Trạng thái kiểm thử

Mã nguồn hiện tại đã đạt **4 nhóm kiểm thử host**: logic rắn, rào chắn, LCD giả lập và LED CA/CC kèm EEPROM cấu hình giả lập. Đã kiểm tra cả 6 tổ hợp và cú pháp toàn firmware qua bộ chuyển cú pháp dành cho host.

Chưa Build All bằng CodeVisionAVR, chạy Proteus hoặc kiểm tra board thật cho phiên bản mới nhất. Kết quả host không thay thế kiểm tra trên MCU.

## 9. Tài liệu giải thích chi tiết từng tính năng

Các tài liệu dưới đây mô tả cách chương trình hiện tại thực hiện từng chức năng, gồm **ví dụ dễ hình dung** và **cách chương trình thực hiện** với các đoạn C, bảng dữ liệu và trường hợp đặc biệt.

| Thứ tự đọc | Tính năng | Nội dung chính |
|---:|---|---|
| 1 | [Menu chọn Level và Map](MENU_CHON_LEVEL_VA_MAP.md) | Trạng thái ứng dụng, con trỏ lựa chọn, xác nhận và chuyển màn hình |
| 2 | [Bản đồ và tường cố định](BAN_DO_VA_TUONG_CO_DINH.md) | Mặt nạ bit, ba bố cục map, kiểm tra vùng xuất phát và liên thông |
| 3 | [Điều khiển keypad](DIEU_KHIEN_KEYPAD.md) | GPIO, quét ma trận, chống dội, nhấn/thả, sự kiện và hướng rẽ chờ |
| 4 | [Di chuyển xuyên biên](DI_XUYEN_BIEN.md) | Bốn cạnh, tọa độ có dấu, va chạm tại nơi đến và quan hệ với vật cản/đảo đầu |
| 5 | [Ăn mồi, cộng điểm và tăng chiều dài](AN_MOI_TANG_DIEM_VA_CHIEU_DAI.md) | Giữ đuôi, dịch mảng, tăng một đốt, tính điểm và tạo mồi mới |
| 6 | [Đảo đầu–đuôi sau khi ăn](DAO_DAU_DUOI_KHI_AN_MOI.md) | Đảo thứ tự các đốt, suy hướng từ cổ mới, xử lý thân cong và xuyên biên |
| 7 | [Sinh vật cản](SINH_VAT_CAN.md) | Thời gian RUNNING, chọn vị trí, vùng an toàn, BFS và ghi nhận thanh chắn |
| 8 | [Chiến thắng — WIN](CHIEN_THANG_WIN.md) | Sức chứa theo map/vật cản, lần ăn cuối và xử lý kết quả |
| 9 | [Chết — GAME OVER](CHET_VA_GAME_OVER.md) | Va chạm, quy tắc ô đuôi, dừng ván, kết quả, Restart và về menu |
| 10 | [Hiển thị trên ba thiết bị](HIEN_THI_NOKIA_LCD_LED7DOAN.md) | Vẽ Nokia, quét LED 7 đoạn, cập nhật LCD từng ký tự và phối hợp trong vòng chính |
| 11 | [Chọn CA/CC và lưu cấu hình LED](CHE_DO_LED_CA_CC.md) | BT1/BT2, chống dội, cực tính segment/digit, EEPROM và khôi phục sau reset |

“Chuyển màn hình” ở đây là đổi giữa menu Level, menu Map, màn chơi và kết quả. Bản hiện tại không tự chuyển sang map khác khi đạt một mốc điểm hoặc thắng ván.
