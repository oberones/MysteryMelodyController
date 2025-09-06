# Mystery Melody Machine - Teensy Firmware Makefile
# Comprehensive build, test, and development workflow automation

# =============================================================================
# Configuration Variables
# =============================================================================

# Project information
PROJECT_NAME := mystery-melody-machine-teensy
FIRMWARE_VERSION := 0.2.0
BUILD_DATE := $(shell date '+%Y-%m-%d %H:%M:%S')

# PlatformIO configuration
PLATFORMIO_ENV := teensy41
PLATFORMIO_ENV_DEBUG := teensy41-debug
PLATFORMIO_ENV_TEST := teensy41-test

# Paths
PROJECT_ROOT := $(CURDIR)
SRC_DIR := $(PROJECT_ROOT)/src
INCLUDE_DIR := $(PROJECT_ROOT)/include
TEST_DIR := $(PROJECT_ROOT)/test
DOCS_DIR := $(PROJECT_ROOT)/docs
BUILD_DIR := $(PROJECT_ROOT)/.pio/build

# Colors for output
RED := \033[31m
GREEN := \033[32m
YELLOW := \033[33m
BLUE := \033[34m
MAGENTA := \033[35m
CYAN := \033[36m
RESET := \033[0m

# =============================================================================
# Default Target
# =============================================================================

.PHONY: help
help: ## Display this help message
	@echo "$(CYAN)Mystery Melody Machine - Teensy Firmware$(RESET)"
	@echo "$(YELLOW)Version: $(FIRMWARE_VERSION)$(RESET)"
	@echo ""
	@echo "$(BLUE)Available targets:$(RESET)"
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  $(GREEN)%-20s$(RESET) %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo ""
	@echo "$(BLUE)Common workflows:$(RESET)"
	@echo "  $(CYAN)make setup$(RESET)          - Initial project setup"
	@echo "  $(CYAN)make build$(RESET)          - Build production firmware"
	@echo "  $(CYAN)make upload$(RESET)         - Build and upload to Teensy"
	@echo "  $(CYAN)make debug$(RESET)          - Build, upload, and monitor debug version"
	@echo "  $(CYAN)make test$(RESET)           - Run all tests"
	@echo "  $(CYAN)make monitor$(RESET)        - Start serial monitor"
	@echo "  $(CYAN)make clean-all$(RESET)      - Clean everything and rebuild"

# =============================================================================
# Setup and Dependencies
# =============================================================================

.PHONY: setup
setup: check-platformio install-deps ## Initial project setup and dependency installation
	@echo "$(GREEN)✓ Project setup complete$(RESET)"

.PHONY: check-platformio
check-platformio: ## Check if PlatformIO is installed
	@echo "$(BLUE)Checking PlatformIO installation...$(RESET)"
	@which pio > /dev/null || (echo "$(RED)Error: PlatformIO not found. Install from https://platformio.org/$(RESET)" && exit 1)
	@echo "$(GREEN)✓ PlatformIO found$(RESET)"

.PHONY: install-deps
install-deps: ## Install project dependencies
	@echo "$(BLUE)Installing dependencies...$(RESET)"
	@pio pkg install
	@echo "$(GREEN)✓ Dependencies installed$(RESET)"

.PHONY: update-deps
update-deps: ## Update all dependencies to latest versions
	@echo "$(BLUE)Updating dependencies...$(RESET)"
	@pio pkg update
	@echo "$(GREEN)✓ Dependencies updated$(RESET)"

# =============================================================================
# Build Targets
# =============================================================================

.PHONY: build
build: ## Build production firmware (MIDI mode)
	@echo "$(BLUE)Building production firmware...$(RESET)"
	@pio run -e $(PLATFORMIO_ENV)
	@echo "$(GREEN)✓ Production build complete$(RESET)"

.PHONY: build-debug
build-debug: ## Build debug firmware (Serial mode)
	@echo "$(BLUE)Building debug firmware...$(RESET)"
	@pio run -e $(PLATFORMIO_ENV_DEBUG)
	@echo "$(GREEN)✓ Debug build complete$(RESET)"

.PHONY: build-test
build-test: ## Build test environment
	@echo "$(BLUE)Building test environment...$(RESET)"
	@pio run -e $(PLATFORMIO_ENV_TEST)
	@echo "$(GREEN)✓ Test build complete$(RESET)"

.PHONY: build-all
build-all: build build-debug build-test ## Build all environments
	@echo "$(GREEN)✓ All builds complete$(RESET)"

.PHONY: size
size: build ## Display firmware size information
	@echo "$(BLUE)Firmware size information:$(RESET)"
	@pio run -e $(PLATFORMIO_ENV) --target size

.PHONY: size-debug
size-debug: build-debug ## Display debug firmware size information
	@echo "$(BLUE)Debug firmware size information:$(RESET)"
	@pio run -e $(PLATFORMIO_ENV_DEBUG) --target size

# =============================================================================
# Upload and Programming
# =============================================================================

.PHONY: upload
upload: build ## Build and upload production firmware to Teensy
	@echo "$(BLUE)Uploading production firmware...$(RESET)"
	@pio run -e $(PLATFORMIO_ENV) --target upload
	@echo "$(GREEN)✓ Production firmware uploaded$(RESET)"

.PHONY: upload-debug
upload-debug: build-debug ## Build and upload debug firmware to Teensy
	@echo "$(BLUE)Uploading debug firmware...$(RESET)"
	@pio run -e $(PLATFORMIO_ENV_DEBUG) --target upload
	@echo "$(GREEN)✓ Debug firmware uploaded$(RESET)"

.PHONY: upload-test
upload-test: build-test ## Build and upload test firmware to Teensy
	@echo "$(BLUE)Uploading test firmware...$(RESET)"
	@pio run -e $(PLATFORMIO_ENV_TEST) --target upload
	@echo "$(GREEN)✓ Test firmware uploaded$(RESET)"

# =============================================================================
# Development Workflows
# =============================================================================

.PHONY: debug
debug: upload-debug monitor-debug ## Complete debug workflow: build, upload, and monitor
	@echo "$(GREEN)✓ Debug session complete$(RESET)"

.PHONY: production
production: upload monitor-production ## Complete production workflow: build, upload, and monitor
	@echo "$(GREEN)✓ Production deployment complete$(RESET)"

.PHONY: quick-test
quick-test: upload-debug ## Quick test cycle: upload debug and briefly monitor
	@echo "$(BLUE)Quick testing (10 seconds)...$(RESET)"
	@timeout 10 pio device monitor -e $(PLATFORMIO_ENV_DEBUG) || true
	@echo "$(GREEN)✓ Quick test complete$(RESET)"

# =============================================================================
# Monitoring and Debugging
# =============================================================================

.PHONY: monitor
monitor: ## Start serial monitor (auto-detect environment)
	@echo "$(BLUE)Starting serial monitor...$(RESET)"
	@pio device monitor

.PHONY: monitor-debug
monitor-debug: ## Start serial monitor for debug environment
	@echo "$(BLUE)Starting debug monitor...$(RESET)"
	@pio device monitor -e $(PLATFORMIO_ENV_DEBUG)

.PHONY: monitor-production
monitor-production: ## Start serial monitor for production environment
	@echo "$(BLUE)Starting production monitor...$(RESET)"
	@echo "$(YELLOW)Note: Production MIDI mode may not show serial output$(RESET)"
	@pio device monitor -e $(PLATFORMIO_ENV)

.PHONY: list-devices
list-devices: ## List available serial devices
	@echo "$(BLUE)Available serial devices:$(RESET)"
	@pio device list

# =============================================================================
# Testing
# =============================================================================

.PHONY: test
test: ## Run all unit tests
	@echo "$(BLUE)Running unit tests...$(RESET)"
	@pio test -e $(PLATFORMIO_ENV_TEST)
	@echo "$(GREEN)✓ All tests completed$(RESET)"

.PHONY: test-verbose
test-verbose: ## Run tests with verbose output
	@echo "$(BLUE)Running tests with verbose output...$(RESET)"
	@pio test -e $(PLATFORMIO_ENV_TEST) -v

.PHONY: test-specific
test-specific: ## Run specific test (usage: make test-specific TEST=test_name)
	@if [ -z "$(TEST)" ]; then \
		echo "$(RED)Error: Please specify TEST=test_name$(RESET)"; \
		exit 1; \
	fi
	@echo "$(BLUE)Running test: $(TEST)$(RESET)"
	@pio test -e $(PLATFORMIO_ENV_TEST) -f $(TEST)

.PHONY: test-hardware
test-hardware: upload-test ## Upload test firmware and run hardware tests
	@echo "$(BLUE)Running hardware tests...$(RESET)"
	@echo "$(YELLOW)Hardware tests require manual verification$(RESET)"
	@echo "$(YELLOW)Monitor output for test results$(RESET)"
	@pio device monitor -e $(PLATFORMIO_ENV_TEST)

# =============================================================================
# Code Quality and Analysis
# =============================================================================

.PHONY: check
check: ## Run static code analysis
	@echo "$(BLUE)Running static code analysis...$(RESET)"
	@pio check
	@echo "$(GREEN)✓ Code analysis complete$(RESET)"

.PHONY: check-verbose
check-verbose: ## Run static code analysis with verbose output
	@echo "$(BLUE)Running detailed code analysis...$(RESET)"
	@pio check --verbose

.PHONY: format
format: ## Format code using clang-format (if available)
	@echo "$(BLUE)Formatting code...$(RESET)"
	@if command -v clang-format >/dev/null 2>&1; then \
		find $(SRC_DIR) $(INCLUDE_DIR) -name "*.cpp" -o -name "*.h" | xargs clang-format -i; \
		echo "$(GREEN)✓ Code formatted$(RESET)"; \
	else \
		echo "$(YELLOW)clang-format not found, skipping$(RESET)"; \
	fi

.PHONY: lint
lint: check ## Alias for check (static analysis)

# =============================================================================
# Documentation
# =============================================================================

.PHONY: docs
docs: ## Generate documentation
	@echo "$(BLUE)Generating documentation...$(RESET)"
	@echo "$(YELLOW)Documentation targets:$(RESET)"
	@echo "  - README.md: $(shell wc -l < README.md) lines"
	@echo "  - SPEC.md: $(shell wc -l < SPEC.md) lines"
	@echo "  - TeensySoftwareRoadmap.md: $(shell wc -l < $(DOCS_DIR)/TeensySoftwareRoadmap.md) lines"
	@echo "$(GREEN)✓ Documentation overview complete$(RESET)"

.PHONY: changelog
changelog: ## Display recent changelog entries
	@echo "$(BLUE)Recent changelog entries:$(RESET)"
	@if [ -f CHANGELOG.md ]; then head -20 CHANGELOG.md; else echo "$(YELLOW)CHANGELOG.md not found$(RESET)"; fi

# =============================================================================
# Maintenance and Cleanup
# =============================================================================

.PHONY: clean
clean: ## Clean build artifacts
	@echo "$(BLUE)Cleaning build artifacts...$(RESET)"
	@pio run --target clean
	@echo "$(GREEN)✓ Build artifacts cleaned$(RESET)"

.PHONY: clean-all
clean-all: clean ## Clean everything including dependencies
	@echo "$(BLUE)Cleaning everything...$(RESET)"
	@rm -rf .pio
	@echo "$(GREEN)✓ Everything cleaned$(RESET)"

.PHONY: rebuild
rebuild: clean build ## Clean and rebuild production firmware
	@echo "$(GREEN)✓ Rebuild complete$(RESET)"

.PHONY: rebuild-all
rebuild-all: clean-all build-all ## Clean everything and rebuild all environments
	@echo "$(GREEN)✓ Complete rebuild finished$(RESET)"

# =============================================================================
# Project Information
# =============================================================================

.PHONY: info
info: ## Display project information
	@echo "$(CYAN)Mystery Melody Machine - Teensy Firmware$(RESET)"
	@echo "$(BLUE)Project Information:$(RESET)"
	@echo "  Name: $(PROJECT_NAME)"
	@echo "  Version: $(FIRMWARE_VERSION)"
	@echo "  Build Date: $(BUILD_DATE)"
	@echo "  Root: $(PROJECT_ROOT)"
	@echo ""
	@echo "$(BLUE)Environment Information:$(RESET)"
	@echo "  Production: $(PLATFORMIO_ENV)"
	@echo "  Debug: $(PLATFORMIO_ENV_DEBUG)"
	@echo "  Test: $(PLATFORMIO_ENV_TEST)"
	@echo ""
	@echo "$(BLUE)Directory Structure:$(RESET)"
	@echo "  Source: $(SRC_DIR)"
	@echo "  Include: $(INCLUDE_DIR)"
	@echo "  Tests: $(TEST_DIR)"
	@echo "  Docs: $(DOCS_DIR)"
	@echo "  Build: $(BUILD_DIR)"

.PHONY: version
version: ## Display version information
	@echo "$(CYAN)Firmware Version: $(FIRMWARE_VERSION)$(RESET)"
	@pio --version

.PHONY: status
status: ## Display project status
	@echo "$(BLUE)Project Status:$(RESET)"
	@echo "  Source files: $(shell find $(SRC_DIR) -name "*.cpp" -o -name "*.h" | wc -l)"
	@echo "  Include files: $(shell find $(INCLUDE_DIR) -name "*.h" 2>/dev/null | wc -l || echo 0)"
	@echo "  Test files: $(shell find $(TEST_DIR) -name "*.cpp" 2>/dev/null | wc -l || echo 0)"
	@echo "  Last build: $(shell if [ -d $(BUILD_DIR) ]; then ls -la $(BUILD_DIR) | head -1; else echo "No builds found"; fi)"

# =============================================================================
# Platform-Specific Targets
# =============================================================================

.PHONY: macos-setup
macos-setup: ## macOS-specific setup instructions
	@echo "$(BLUE)macOS Setup Instructions:$(RESET)"
	@echo "1. Install PlatformIO:"
	@echo "   $(CYAN)pip install platformio$(RESET)"
	@echo "2. Install Teensy Loader Application from PJRC"
	@echo "3. Add user to dialout group (if needed):"
	@echo "   $(CYAN)sudo dseditgroup -o edit -a \$$USER -t user dialout$(RESET)"
	@echo "4. Verify Teensy connection:"
	@echo "   $(CYAN)make list-devices$(RESET)"

# =============================================================================
# Development Helpers
# =============================================================================

.PHONY: watch
watch: ## Watch for file changes and rebuild (requires entr)
	@echo "$(BLUE)Watching for changes (requires 'entr')...$(RESET)"
	@if command -v entr >/dev/null 2>&1; then \
		find $(SRC_DIR) $(INCLUDE_DIR) -name "*.cpp" -o -name "*.h" | entr -c make build; \
	else \
		echo "$(RED)Error: 'entr' not found. Install with: brew install entr$(RESET)"; \
	fi

.PHONY: watch-debug
watch-debug: ## Watch for file changes and rebuild debug version
	@echo "$(BLUE)Watching for debug changes...$(RESET)"
	@if command -v entr >/dev/null 2>&1; then \
		find $(SRC_DIR) $(INCLUDE_DIR) -name "*.cpp" -o -name "*.h" | entr -c make build-debug; \
	else \
		echo "$(RED)Error: 'entr' not found. Install with: brew install entr$(RESET)"; \
	fi

.PHONY: deploy
deploy: test build upload ## Complete deployment workflow: test, build, and upload
	@echo "$(GREEN)✓ Deployment complete$(RESET)"

.PHONY: ci
ci: check test build-all ## Continuous integration workflow
	@echo "$(GREEN)✓ CI workflow complete$(RESET)"

# =============================================================================
# Advanced Targets
# =============================================================================

.PHONY: benchmark
benchmark: upload-debug ## Run performance benchmarks
	@echo "$(BLUE)Running performance benchmarks...$(RESET)"
	@echo "$(YELLOW)Monitor serial output for timing data$(RESET)"
	@timeout 30 pio device monitor -e $(PLATFORMIO_ENV_DEBUG) | grep -E "(Loop|Portal|MIDI)" || true
	@echo "$(GREEN)✓ Benchmark data collected$(RESET)"

.PHONY: soak-test
soak-test: upload-debug ## Run soak test (extended operation test)
	@echo "$(BLUE)Starting soak test (10 minutes)...$(RESET)"
	@echo "$(YELLOW)Monitoring for stability issues...$(RESET)"
	@timeout 600 pio device monitor -e $(PLATFORMIO_ENV_DEBUG) || echo "$(GREEN)✓ Soak test completed$(RESET)"

.PHONY: factory-reset
factory-reset: clean-all setup build upload ## Complete factory reset and rebuild
	@echo "$(GREEN)✓ Factory reset complete$(RESET)"

# =============================================================================
# Configuration Management
# =============================================================================

.PHONY: config-show
config-show: ## Show current configuration
	@echo "$(BLUE)Current Configuration:$(RESET)"
	@cat platformio.ini

.PHONY: config-backup
config-backup: ## Backup current configuration
	@echo "$(BLUE)Backing up configuration...$(RESET)"
	@cp platformio.ini platformio.ini.backup.$(shell date +%Y%m%d_%H%M%S)
	@echo "$(GREEN)✓ Configuration backed up$(RESET)"

# =============================================================================
# Makefile Validation
# =============================================================================

.PHONY: makefile-check
makefile-check: ## Validate Makefile syntax
	@echo "$(BLUE)Checking Makefile syntax...$(RESET)"
	@make -n help > /dev/null && echo "$(GREEN)✓ Makefile syntax OK$(RESET)" || echo "$(RED)✗ Makefile syntax error$(RESET)"

# =============================================================================
# Special Targets
# =============================================================================

# Prevent make from deleting intermediate files
.SECONDARY:

# Ensure certain targets always run
.PHONY: help setup check-platformio install-deps update-deps build build-debug build-test build-all
.PHONY: size size-debug upload upload-debug upload-test debug production quick-test
.PHONY: monitor monitor-debug monitor-production list-devices test test-verbose test-specific test-hardware
.PHONY: check check-verbose format lint docs changelog clean clean-all rebuild rebuild-all
.PHONY: info version status macos-setup watch watch-debug deploy ci benchmark soak-test factory-reset
.PHONY: config-show config-backup makefile-check

# Default target
.DEFAULT_GOAL := help
