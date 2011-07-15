

set(arm_SOURCES
	"arm.cpp"
	)

add_library(arm MODULE ${arm_SOURCES})
install(TARGETS arm DESTINATION lib)
