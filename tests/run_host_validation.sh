#!/bin/sh
# Host-only checks. Never add tests/mock to the CodeVisionAVR include path.
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output_dir=$(mktemp -d "${TMPDIR:-/tmp}/snake-host-validation.XXXXXX")
host_cc=${CC:-cc}

"$host_cc" -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/game/game_map.c" "$repo_dir/game/game_mode.c" \
    "$repo_dir/game/bonus_food.c" "$repo_dir/game/dynamic_obstacle.c" \
    "$repo_dir/tests/host_snake.c" -o "$output_dir/host_snake"
"$output_dir/host_snake"

"$host_cc" -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/game/game_map.c" "$repo_dir/game/game_mode.c" \
    "$repo_dir/game/bonus_food.c" "$repo_dir/tests/host_dynamic.c" \
    -o "$output_dir/host_dynamic"
"$output_dir/host_dynamic"

"$host_cc" -DHOST_TEST -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/game/game_map.c" \
    "$repo_dir/game/game_mode.c" \
    "$repo_dir/game/game_menu.c" \
    "$repo_dir/storage/highscore_eeprom.c" \
    "$repo_dir/tests/host_eeprom.c" -o "$output_dir/host_eeprom"
"$output_dir/host_eeprom"

"$host_cc" -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/drivers/lcd1602.c" \
    "$repo_dir/tests/host_lcd.c" -o "$output_dir/host_lcd"
"$output_dir/host_lcd"

echo "Host validation binaries: $output_dir"
