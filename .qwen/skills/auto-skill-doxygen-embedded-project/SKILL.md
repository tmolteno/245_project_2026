---
name: doxygen-embedded-project
description: Set up Doxygen HTML documentation for a C/C++ embedded PlatformIO project with namespace-based libraries — minimal config, no Doxygen-comment requirements, auto-extracted API docs
source: auto-skill
extracted_at: '2026-07-16T11:30:00.000Z'
---

# Doxygen for Embedded PlatformIO Projects

Set up Doxygen to generate HTML API documentation for a PlatformIO-based embedded C/C++
project where the libraries live under `lib/` and use namespaces rather than classes.
The goal is documentation that works **without** any Doxygen-formatted comments in the
source — useful for rapid bootstrapping or codebases that use plain comments.

## Install doxygen

```bash
sudo apt install doxygen
```

## Directory layout

```
project/
├── os/
│   ├── Doxyfile           # config lives outside the build artifacts
│   └── doc/
│       └── html/           # generated output (committed to repo)
├── lib/
│   ├── PHSI245_HAL/
│   ├── phsi245_gfx/
│   ├── phsi245_input/
│   └── ...
└── src/
    └── main.cpp
```

## Minimal Doxyfile

Create `os/Doxyfile` with only the settings that matter. No need to run `doxygen -g` —
start minimal and add as needed:

```
PROJECT_NAME           = "Project Name"
PROJECT_BRIEF          = "One-line description"
OUTPUT_DIRECTORY       = os/doc

OPTIMIZE_OUTPUT_FOR_C  = YES         # C-style code, namespaces treated as modules
CASE_SENSE_NAMES       = YES
JAVADOC_AUTOBRIEF      = YES

EXTRACT_ALL            = YES         # document everything, even without /** comments */
EXTRACT_STATIC         = YES         # include static file-scope helpers
SORT_MEMBER_DOCS       = YES

INPUT                  = lib/LIB_A \
                         lib/LIB_B \
                         lib/LIB_C

RECURSIVE              = NO          # explicit dirs are safer than recursive
FILE_PATTERNS          = *.h *.cpp

GENERATE_HTML          = YES
HTML_OUTPUT            = html

GENERATE_LATEX         = NO          # embedded devs want HTML, not PDF
GENERATE_XML           = NO
GENERATE_MAN           = NO

HAVE_DOT               = NO          # don't require Graphviz
```

### Key settings explained

| Setting | Why |
|---------|-----|
| `EXTRACT_ALL = YES` | Documents all functions/macros/variables even without `/** */` doc comments |
| `EXTRACT_STATIC = YES` | Pulls in file-scope `static` helpers — critical for embedded code where many helpers are static |
| `OPTIMIZE_OUTPUT_FOR_C = YES` | Treats namespaces as labeled sections rather than C++ classes; matches the `namespace gfx { … }` pattern common in embedded libraries |
| `RECURSIVE = NO` | Explicit `INPUT` directories avoid picking up third-party libs, build artifacts, or `src_games/` |
| `HAVE_DOT = NO` | Removes the Graphviz dependency; class diagrams aren't useful for namespace-based code anyway |

## Regenerate command

```bash
doxygen os/Doxyfile
```

This writes output to `os/doc/html/`. Open `os/doc/html/index.html` in a browser.

## Doxygen version compatibility

Doxygen 1.9+ introduces and deprecates tags across versions. When moving between versions
you may see warnings like:

```
warning: Tag 'HTML_TIMESTAMP' has become obsolete.
warning: Tag 'CLASS_DIAGRAMS' has become obsolete.
```

Just remove the obsolete tag from the Doxyfile. The generated docs are unaffected. Run
`doxygen -u os/Doxyfile` to auto-upgrade the config if you want to see all changes at once.

## Git considerations

- Commit `os/Doxyfile` (it's the source of truth for the doc build)
- Commit `os/doc/html/` if you want docs browsable on GitHub/GitLab without a build step
- Add `os/doc/html/` to `.gitignore` if you prefer docs to be regenerated locally
- The `.pio/` build directory should already be gitignored

## Updating when libraries change

Add new libraries to the `INPUT` list in the Doxyfile and regenerate. The `AGENTS.md` or
`QWEN.md` at the repo root should record the regeneration command and which directories
feed into the docs, so anyone touching a library knows to rebuild.

## Namespace output

With `OPTIMIZE_OUTPUT_FOR_C = YES`, each namespace (`gfx`, `fat`, `sd`, `input`, `led`,
etc.) gets its own page under the Namespaces tab. Functions appear as free functions under
their namespace, not as class methods. This matches the mental model of C-style embedded
libraries that use namespaces for scoping rather than OOP classes.
