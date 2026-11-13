# PHP diseval extension

The purpose of the diseval php extension is to disable `eval()` in php.
`eval` is a language construct, not a function, so `disable_functions=eval` does
nothing. Suhosin could disable eval in older versions of php. Diseval disables eval
in php5 and php7 with version 0.1, and in php8 with version 0.2 and later.

When this php extension is enabled, the following yields a fatal error and the
string is never compiled or run:

```php
eval("echo 44;");
```

    PHP Fatal error:  DISEVAL - Use of eval is forbidden in /path/to/script.php on line 4

On PHP 5/7 (diseval 0.1) the extension also blocked `create_function()`, the `e`
modifier of `mb_ereg_replace()` and string `assert()`. PHP 8 removed all three from
the language, so `eval()` is the only one left to block.

## On/off switch

```ini
; php.ini / conf.d/diseval.ini  (default: 1)
extension = diseval.so
diseval.enabled = 0
```

`diseval.enabled` is `PHP_INI_SYSTEM`. You can set it in php.ini, in a conf.d file,
in the Apache server config, or in a php-fpm pool with
`php_admin_value[diseval.enabled] = 0`. Scripts cannot change it with `ini_set()`
or `.user.ini`, so it cannot be used to bypass the protection. A common setup keeps
it on for production FPM pools and turns it off for development, CLI tools or a
pool that runs an application which needs `eval()`.

## How it works

From 0.3, diseval hooks the engine's `zend_compile_string()` and keeps the
previous hook in the chain. It refuses to compile the string only when the
executing PHP code is at an `eval()` opcode (`ZEND_INCLUDE_OR_EVAL` in `ZEND_EVAL`
mode). The string is never compiled, so it never runs.

Up to 0.2, diseval replaced `zend_execute_ex()` and aborted when an
`ZEND_EVAL_CODE` op array started running. That caused three problems:

* **It also blocked `zend_eval_string()` called from C.** `php -r`, `php -a`,
  phpdbg and Xdebug (watches, conditional breakpoints, "evaluate") all use it, so
  `php -r 'echo 1;'` died with the diseval error.
* **It turned off the OPcache JIT** with "JIT is incompatible with third party
  extensions that override zend_execute_ex(). JIT disabled."
* **It slowed down every userland function call.** Overriding `zend_execute_ex`
  makes the VM run each call through a nested C call instead of inline, which also
  uses more C stack on deep recursion.

The 0.3 hook runs only when a string is compiled, so ordinary code pays nothing
and the JIT stays on. `testing/run-tests.sh` checks all of this.

## Should you disable eval? Background and trade-offs

PHP core has discussed disabling eval more than once and has not settled it:

* [php-src#4084](https://github.com/php/php-src/pull/4084) made
  `disable_functions=eval` work by replacing `zend_compile_string`. It was merged
  into 7.4 and then reverted. Derick Rethans objected that it broke Xdebug, which
  calls `zend_eval_string()`, and that the hook could not be chained cleanly. Joe
  Watkins argued that an attacker can use `include` instead. The bug was later
  reopened ([bug #62397](https://bugs.php.net/bug.php?id=62397)) with the note that
  it needs an RFC.
* [externals.io #107860](https://externals.io/message/107860) ("Let's allow eval()
  to be turned off in PHP 8") did not reach a decision. The only agreement was
  that any switch must be opt-in.

Diseval is built to avoid the technical objections. It is opt-in (load the
extension, turn it off per pool), it chains the hook, and it leaves C-level
`zend_eval_string()` alone, so Xdebug, `php -r` and `php -a` keep working.

**What it protects against, and what not**

Diseval stops *string injection into an existing `eval()`*. If an application, plugin
or dependency ever passes attacker-controlled data to `eval()`, that is normally
remote code execution. With diseval it is a fatal error.

It does **not** stop code that is *already running*. Any PHP code an attacker
manages to execute does not need `eval()`. Without writing any file, and with
`allow_url_include=Off`, it can still run an arbitrary string as PHP code by:

* registering its own stream wrapper and including from it:
  `stream_wrapper_register('x', ...)` + `include 'x://...'`. User-registered
  wrappers are not "URL" wrappers, so `allow_url_include` does not apply;
* including a writable temporary or upload file through a `php://filter`
  conversion, e.g. `include 'php://filter/read=convert.base64-decode/resource=/tmp/...'`;
* with FFI enabled, calling the engine's C `zend_eval_string()` directly.

These paths existed with every previous diseval version too. A ten-line shim gets
eval-based malware running again, so treat diseval as one layer, not a sandbox.

**Pros**

* It blocks the classic remote-code-execution primitive: a request value that
  reaches `eval()`, or "fetch a string, `base64_decode` it, `eval` it" in a
  vulnerable plugin. With diseval this is a fatal error, not code execution.
* Off-the-shelf web shells, droppers and obfuscated malware that call `eval()`
  directly stop working until they are rewritten.
* It is defence in depth for code you do not fully control, such as plugins,
  themes and dependencies.
* It costs nothing at runtime and does not affect the JIT, OPcache, Xdebug or the
  CLI.

**Cons and limits**

* It is not a sandbox (see above).
* C extensions can still compile strings. diseval only blocks the `eval()` language
  construct in PHP code, so C callers of `zend_eval_string()` are allowed by
  design. In PHP core those are only `php -r`/`-B`/`-R`/`-E`, `php -a` and phpdbg,
  none of which a web request can reach. Third-party extensions that compile
  strings for userland, such as runkit7 or uopz, and FFI (above) are not covered.
* Some legitimate code uses `eval()`, so test your application before enabling
  diseval:
  * PHPUnit mock objects.
  * Twig and some Symfony components as fallbacks when no cache is available.
  * Some code generators and ORM proxies without a cache.
  * PsySH / Laravel Tinker.

  Use `diseval.enabled=0` for pools or CLI configs that need it.
* An `eval()` is a fatal error, not a catchable exception. This is deliberate:
  failing hard makes sure blocked code never half-runs.
* Another extension that installs a `zend_compile_string` hook later and does not
  chain to the previous one would bypass diseval. This is rare.

**Recommended hardening alongside diseval** (web SAPIs; check what your
application needs first):

```ini
allow_url_include = Off
ffi.enable = Off              ; the default "preload" is already off for web requests
disable_functions = stream_wrapper_register,stream_wrapper_restore
```

Also keep code directories read-only for the PHP user, and keep upload and
temporary directories out of reach of `include`, e.g. with `open_basedir` or by
monitoring them. `php://filter` over a writable file stays possible as long as
PHP can include a file it can write.

## PHP compatibility

| PHP | NTS | ZTS | Notes |
|-----|-----|-----|-------|
| 5.x, 7.x | - | - | use diseval 0.1 |
| 8.0 | tested | tested | no JIT in the tested builds |
| 8.1 | tested | tested | also tested with OPcache function and tracing JIT |
| 8.2 | tested | tested | also tested with OPcache function and tracing JIT |
| 8.3 | tested | tested | also tested with OPcache function and tracing JIT |
| 8.4 | tested | tested | also tested with OPcache function and tracing JIT |
| 8.5 | tested | tested | also tested with OPcache function and tracing JIT |
| 8.6 | tested | tested | 8.6.0RC5, the release candidate at the time of testing |

Run the tests with `testing/run-tests.sh /path/to/php /path/to/diseval.so`. To
run them in Docker for any version:

    docker build -f testing/Dockerfile.test --build-arg PHP=8.3-cli .

## Installation with PIE (Composer)

Each GitHub release has prebuilt binaries attached, so no compiler is needed:

* Linux: glibc and musl, x86_64 and arm64 (musl on x86_64 only), NTS and ZTS,
  PHP 8.1-8.6.
* macOS: arm64, NTS, PHP 8.1-8.5.

PHP 8.0 is end-of-life, so it gets no prebuilt binaries; PIE builds it from source.

```bash
pie install mk-j/diseval
echo "diseval.enabled = 1" >> "$(php -r 'echo PHP_CONFIG_FILE_SCAN_DIR;')/diseval.ini"   # optional, 1 is the default
```

If there is no matching binary, PIE builds from source, which needs
`phpize`/`php-dev` and a C compiler. The PIE package does not support Windows.

Every release also carries a `SHA256SUMS` file and a GitHub build-provenance
attestation for each binary. To check that a zip was built by this repository's
release workflow:

```bash
gh attestation verify php_diseval-<version>_php8.4-x86_64-linux-glibc.zip --repo mk-j/PHP_diseval_extension
```

## Building

See `INSTALL`: `cd source && phpize && ./configure --enable-diseval && make`.
