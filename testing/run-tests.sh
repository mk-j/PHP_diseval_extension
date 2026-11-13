#!/usr/bin/env bash
# usage: run-tests.sh /path/to/php /path/to/diseval.so
PHP=${1:-php}; SO=${2:-modules/diseval.so}; D=$(dirname "$0"); rc=0
BLOCKED="DISEVAL - Use of eval is forbidden"
# Both helpers also fail when PHP was killed by a signal (exit status 129-192, e.g. 139 =
# SIGSEGV; 255 is PHP's normal fatal-error exit), so a message printed just before a crash
# during bailout can't produce a false "ok".
crashed() { [ "$1" -gt 128 ] && [ "$1" -le 192 ]; }
chk() { # name expected-regex cmd...
  local n=$1 e=$2 out st; shift 2; out=$("$@" 2>&1); st=$?
  if crashed $st; then echo "FAIL $n: crashed (exit $st): $out"; rc=1
  elif echo "$out" | grep -Eq "$e"; then echo "ok   $n"; else echo "FAIL $n: $out"; rc=1; fi
}
chk_not() { # name forbidden-regex cmd...
  local n=$1 e=$2 out st; shift 2; out=$("$@" 2>&1); st=$?
  if crashed $st; then echo "FAIL $n: crashed (exit $st): $out"; rc=1
  elif echo "$out" | grep -Eq "$e"; then echo "FAIL $n: $out"; rc=1; else echo "ok   $n"; fi
}
base=("$PHP" -n -d display_errors=On -d "extension=$SO")

chk "loaded"                       "^diseval$"   "${base[@]}" -m
chk "eval blocked (default)"       "$BLOCKED"    "${base[@]}" "$D/fail1-eval.php"
chk "eval blocked (enabled=1)"     "$BLOCKED"    "${base[@]}" -d diseval.enabled=1 "$D/fail1-eval.php"
chk "eval allowed (enabled=0)"     "^s$"         "${base[@]}" -d diseval.enabled=0 "$D/fail1-eval.php"
chk "ini_set cannot disable"       "$BLOCKED"    "${base[@]}" "$D/fail5-ini-set.php"
chk_not "ini_set: no eval output"  "eval ran"    "${base[@]}" "$D/fail5-ini-set.php"
chk "nested eval blocked"          "$BLOCKED"    "${base[@]}" "$D/fail6-nested.php"
chk "normal code unaffected"       "^6767$"      "${base[@]}" "$D/ok1-no-eval.php"
# zend_eval_string() from C (php -r, php -a, Xdebug watches) must keep working.
chk "php -r still works"           "^r-ok$"      "${base[@]}" -r 'echo "r-ok";'
chk "php -r with eval blocked"     "$BLOCKED"    "${base[@]}" -r 'eval("echo 1;");'

# opcache + JIT: the hook must not disable JIT and eval must stay blocked in JIT-ed code.
if "$PHP" -n -d zend_extension=opcache -m 2>/dev/null | grep -qi "zend opcache" \
   || "$PHP" -n -m 2>/dev/null | grep -qi "zend opcache"; then
  OPC=("$PHP" -n -d display_errors=On)
  "$PHP" -n -m | grep -qi "zend opcache" || OPC+=(-d zend_extension=opcache)
  OPC+=(-d opcache.enable_cli=1 -d opcache.jit=tracing -d opcache.jit_buffer_size=16M)
  JIT='echo (opcache_get_status()["jit"]["on"] ?? false) ? "jit-on" : "jit-off";'
  if "${OPC[@]}" -r "$JIT" 2>&1 | grep -q "jit-on"; then
    chk     "JIT stays enabled"       "^jit-on$"      "${OPC[@]}" -d "extension=$SO" -r "$JIT"
    chk_not "no 'JIT disabled' warn"  "JIT disabled"  "${OPC[@]}" -d "extension=$SO" -r "$JIT"
    # Run the eval opline as JIT-compiled code: function JIT (1205: compile all functions on
    # load) and tracing JIT (1255: after the hot loop). Some old builds (PHP 8.0/8.1
    # ZTS+musl) crash on these scripts under JIT even without diseval loaded; that is an
    # engine bug, so a case is only checked when it runs cleanly without the extension.
    for mode in 1205 1255; do
      for script in fail7-jit-hot.php fail6-nested.php; do
        "${OPC[@]}" -d opcache.jit=$mode "$D/$script" >/dev/null 2>&1; base_rc=$?
        if ! crashed $base_rc; then
          chk     "eval blocked, jit=$mode, $script"    "$BLOCKED"  "${OPC[@]}" -d opcache.jit=$mode -d "extension=$SO" "$D/$script"
          chk_not "no eval output, jit=$mode, $script"  "eval ran"  "${OPC[@]}" -d opcache.jit=$mode -d "extension=$SO" "$D/$script"
        else
          echo "skip jit=$mode $script (PHP itself crashes on it under JIT, exit $base_rc)"
        fi
      done
    done
  else
    echo "skip JIT tests (JIT not available on this build)"
  fi
  chk "eval blocked with opcache"   "$BLOCKED"  "${OPC[@]}" -d opcache.jit=disable -d "extension=$SO" "$D/fail6-nested.php"
else
  echo "skip opcache tests (opcache not available)"
fi
exit $rc
