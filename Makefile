.PHONY: upload upload-v1 build build-v1 clean

ENV ?= v2

build:
	pio run -e $(ENV)

upload: build
	pio run -e $(ENV) -t upload

build-v1:
	pio run -e v1

upload-v1: build-v1
	pio run -e v1 -t upload

clean:
	pio run -t clean
