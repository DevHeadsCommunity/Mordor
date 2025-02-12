# Copyright (c) 2025 DevHeads Community
# MIT License

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_DRV8305 drv8305)
