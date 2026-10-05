#!/bin/sh
# Guards: (a) shared mutable static across cars - 79b87f9 static last/second_last trackers polluted all cars;
# (b) per-car hot-path log_info spam - stripped twice d4cea9d,b528e98, PROJECT_RULES "remove before committing".
fail=0
if grep -rnE 'static (size_t|int|float|bool) [a-z_]+;|^[a-z_]+ [A-Za-z]+::[a-z_]+ =' src/systems/*.h; then echo "mutable static in systems forbidden - put state on the component (RoadFollowing)"; fail=1; fi
if grep -n "log_info" src/systems/MazeTraversal.h src/systems/LoopDetection.h; then echo "hot-path log_info forbidden in traversal/loop systems"; fail=1; fi
[ $fail -eq 0 ] && echo "check_systems_guards: ok"; exit $fail
