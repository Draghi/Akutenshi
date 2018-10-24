include_guard(GLOBAL)

# ################ #
# # Include Dirs # #
# ################ #

sugar_include(akasset)
sugar_include(akcommon)
sugar_include(akengine)
sugar_include(akgame)
sugar_include(akinput)
sugar_include(akmath)
sugar_include(akrender)
sugar_include(aksound)

# ################ #
# # Source Files # #
# ################ #

sugar_files(AK_SOURCE 
	main.cpp 
	game.cpp 
	imp.cpp
)
