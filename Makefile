.PHONY: upload upload-v1 build build-v1 clean build-game

ENV ?= v2

build:
	pio run -e $(ENV)

upload: build
	pio run -e $(ENV) -t upload

build-v1:
	pio run -e v1

upload-v1: build-v1
	pio run -e v1 -t upload

# Build a game from src_games/ (e.g. make build-game GAME=OSDemo)
build-game:
	@[ "${GAME}" ] || (echo "Usage: make build-game GAME=<name>  (e.g. GAME=OSDemo)"; exit 1)
	@cp platformio.ini platformio.ini.bak
	@printf '\nplatformio.src_dir = src_games/%s\n' "${GAME}" >> platformio.ini
	pio run -e $(ENV) || (mv platformio.ini.bak platformio.ini; exit 1)
	@mv platformio.ini.bak platformio.ini

clean:
	pio run -t clean
