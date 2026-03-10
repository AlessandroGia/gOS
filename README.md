# GOS

bootloader UEFI POC

## Requirements

- Make
- Docker
- Docker Compose

- QEMU instelled on host system
- OVMF firmware file

## Setup

Place OVFM firware (OVMF_CODE.fd) in assets folder.
Builds are executed inside Docker, while QEMU is run on the host system.

## Usage

### Build the Docker image

Run this the first time, or whenever the `Dockerfile` changes:

```bash
make image
```

### Build the project

```bash
make build
```

This generates the UEFI binary in the build directory.

### Run the VM

```bash
make run
```

This prepares the UEFI boot path and starts QEMU with OVMF.

### Clean generated files

```bash
make clean
```

## License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for details.
