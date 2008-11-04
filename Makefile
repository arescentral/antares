.PHONY: all build clean install test

all build clean install test:
	@echo 'Forwarding request to `jam '$@'`'...
	@echo 'In the future, please use `jam` with this project instead of `make`'
	@exec jam $@
