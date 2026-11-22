# Các tính năng của Snake Game ATmega128

Tài liệu mô tả bản **8 file mã nguồn gốc** hiện tại. Bản rút gọn trong `export/codevision/` chưa cập nhật các thay đổi này.

## 1. Hai chế độ chơi

| Chế độ | Tốc độ | Đặc điểm |
|---|---|---|
| NORMAL | 300 ms/ô | Rắn đi qua một cạnh màn hình sẽ xuất hiện ở cạnh đối diện |
| HARD | 250 ms/ô | Xuyên biên và đảo đầu–đuôi sau khi ăn mồi, nếu chưa thắng |

Mặc định chọn NORMAL. Luồng bắt đầu: **chọn NORMAL/HARD → chọn map → chơi**, không có màn chọn STANDARD/SURVIVAL riêng.

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
- Tạm dừng không tính thời gian sinh rào chắn; chơi lại xóa toàn bộ rào chắn và đặt lại lịch.

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
| 5 | Xác nhận | Tạm dừng / tiếp tục |
| 9 | — | Chơi lại |
| 1 | Từ menu Map về menu chế độ | Về menu chế độ sau khi thắng hoặc thua |

Keypad có chống dội phím; giữ phím không tạo liên tiếp nhiều sự kiện nhấn.

## 6. Tạm dừng, chơi lại, thắng và thua

- **Tạm dừng:** giữ vị trí rắn, hướng, điểm, mồi và rào chắn; dừng di chuyển và thời gian sinh rào chắn.
- **Tiếp tục:** chơi tiếp ván hiện tại.
- **Chơi lại:** giữ chế độ và map đã chọn, đưa rắn về 3 đốt, điểm về 0, xóa rào chắn và đặt lại thời gian.
- **Game Over:** đầu rắn đâm vào thân, tường cố định hoặc rào chắn.
- **Win:** rắn chiếm hết các ô chơi hợp lệ còn lại.
- Màn hình cuối ván hiển thị điểm và hướng dẫn chơi lại hoặc về menu.

## 7. Hiển thị trên ba thiết bị

| Thiết bị | Nội dung |
|---|---|
| Nokia 5110, 84×48 pixel | Menu, vùng chơi 14×8 ô, rắn, mồi, tường, rào chắn và kết quả |
| LED 7 đoạn, 4 chữ số | Điểm hiện tại |
| LCD 16×2 | Chế độ, map, số rào chắn, trạng thái PLAYING/PAUSED, kỷ lục trong menu và kết quả cuối ván |

## 8. Lưu kỷ lục bằng EEPROM

- Lưu kỷ lục riêng cho **6 tổ hợp NORMAL/HARD × 3 map**.
- Khi thắng hoặc thua, chỉ ghi EEPROM nếu điểm cao hơn kỷ lục hiện có.
- Đọc lại kỷ lục khi khởi động; chơi lại không xóa kỷ lục.
- Dùng hai slot, CRC và dấu xác nhận bản ghi; hỗ trợ đọc định dạng dữ liệu cũ.
- Kỷ lục gameplay hiện tại dùng nhóm SURVIVAL cũ vì cùng luật rào chắn; các nhóm dữ liệu cũ khác được bảo toàn.

## 9. Trạng thái kiểm thử

Mã nguồn hiện tại đã đạt **4 nhóm kiểm thử host**: logic rắn, rào chắn, EEPROM giả lập và LCD giả lập. Đã kiểm tra cả 6 tổ hợp và cú pháp toàn firmware qua bộ chuyển cú pháp dành cho host.

Chưa Build All bằng CodeVisionAVR, chạy Proteus hoặc kiểm tra board thật cho phiên bản mới nhất. Kết quả host không thay thế kiểm tra trên MCU.
