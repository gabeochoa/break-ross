# CORRECT — break-ross
Gate: `make check && make` (clean build if flags changed — see NOTES.md).

## Classes (each seen >=2x)
| # | Class | Evidence (2+) | Level / why | Guard (fails on past) | Commit |
|---|---|---|---|---|---|
|1|Cached/duplicate reveal state drifts from grid|f1ce7ca % computed from FogOfWar (wrong owner); 666fb0c single-owner refactor left cached `reveal_percentage` + Territory manual `=0` reset; 1251a7c/573b729 % fixes|Architecture/types — field deleted, value is derived-only `get_reveal_percentage()`, no cache to drift|`scripts/check_no_cached_reveal.sh`; past field/reset lines -> exit 1; `make` green|d571a55|
|2a|Shared mutable `static` in a system pollutes every entity|79b87f9 static last/second_last trackers (one car polluted others, dead-end freeze); singleton/static-state corrections 799aa9f|Lint naming fix — state must live on component (`RoadFollowing.segment_history`)|`scripts/check_systems_guards.sh` vs pre-79b87f9 MazeTraversal.h -> exit 1|560cc84|
|2b|Per-car hot-path `log_info` spam stripped repeatedly|d4cea9d traversal/loop, b528e98 spawn/purchase; PROJECT_RULES already said remove — docs failed|Lint — rule in PROJECT_RULES was docs-last and ignored; now `make check`|same script forbids log_info in MazeTraversal/LoopDetection|560cc84|

## Rule table
| Do | Never |
|---|---|
|State per car on its component; reveal % via `get_reveal_percentage()`|Add `static` mutable/system fields, cached % fields, or a second reveal grid (FogOfWar=reachability only)|
|`make check` + build before commit; one-off events may log| `log_info` in traversal/loop hot paths; commit debug logs (PROJECT_RULES)|
