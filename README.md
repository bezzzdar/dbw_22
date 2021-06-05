# implementation of telegram bot for winter session of school "dialogue"

## structure

- `db/` - directory for bot database
- `src/` - directory for bot implementation
- `lib/` - directory for 3d party libraries
- `scripts/` - directory for custom shell scripts. to install manually, read through `scripts/install-deps.sh`

## use

- run `# ./install-deps.sh` from `scripts/` directory to install all prerequisites
- make directory `build/` in project folder
- run `cmake ../ && make -j` from `build/` folder. this shall create bot executable `build/src/bot`
- bot's arguments - `hostname`, `username`, `password` - parameters to connect to the database
