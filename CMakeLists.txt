cmake_minimum_required(VERSION 3.25)

#project(vexed_chroma)

project(Haven)

add_executable(Haven src/main.cpp src/haven.cpp src/haven/world.cpp)
target_link_libraries(Haven PRIVATE beaver)
target_compile_definitions(Haven PUBLIC
				DATA_PATH="${Haven_SOURCE_DIR}/data/"
				ASSETS_PATH="${Haven_SOURCE_DIR}/assets/"
				IMAGES_PATH="${Haven_SOURCE_DIR}/assets/images/"
				TILESETS_PATH="${Haven_SOURCE_DIR}/assets/images/tilesets/"
				AUDIOS_PATH="${Haven_SOURCE_DIR}/assets/audios/"
				FONTS_PATH="${Haven_SOURCE_DIR}/assets/fonts/")

