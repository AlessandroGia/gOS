image:
	docker compose build

build:
	docker compose run --rm uefi-build ./scripts/build.sh

run:
	docker compose run --rm uefi-build ./scripts/run.sh
