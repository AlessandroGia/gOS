.PHONY: image build run

ifeq ($(OS),Windows_NT)
    RUN_CMD = powershell -ExecutionPolicy Bypass -File .\scripts\run.ps1
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        RUN_CMD = sh ./scripts/run.sh
    endif
    ifeq ($(UNAME_S),Linux)
        RUN_CMD = bash ./scripts/run.sh
    endif
endif

dev-build:
    docker build --platform=linux/amd64 -f .devcontainer/dev.Dockerfile -t gos-dev-amd64 .

image:
	docker compose build

build:
	docker compose run --rm gos bash ./scripts/build.sh

br: build run

run:
	$(RUN_CMD)

clean:
	docker compose run --rm gos bash ./scripts/clean.sh
