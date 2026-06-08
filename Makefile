.PHONY: protos help

help:
	@echo "Available targets:"
	@echo "  protos  - Generate NanoPB protobuf code"
	@echo ""
	@echo "Note: Activate virtualenv first: source .venv/bin/activate"

protos:
	@if [ -z "$$VIRTUAL_ENV" ]; then \
		echo "Error: No virtualenv activated. Run: source .venv/bin/activate"; \
		exit 1; \
	fi
	@echo "Generating NanoPB protobuf code..."
	@mkdir -p protos/shared/generated/cpp
	@NANOPB_GEN=$$(find .pio/libdeps -name nanopb_generator.py -type f | head -1); \
	if [ -z "$$NANOPB_GEN" ]; then \
		echo "Error: nanopb_generator.py not found. Run 'pio lib install' first."; \
		exit 1; \
	fi; \
	for proto in protos/shared/*.proto; do \
		python3 $$NANOPB_GEN \
			--output-dir=protos/shared/generated/cpp \
			--options-file=protos/shared/$$(basename $$proto .proto).options \
			-Iprotos/shared \
			$$proto; \
	done
	@echo "Done!"

