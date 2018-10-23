include_guard(GLOBAL)

# ################ #
# # Include Dirs # #
# ################ #

sugar_include(AkAsset)
sugar_include(AkCommon)
sugar_include(AkEngine)
sugar_include(AkGame)
sugar_include(AkInput)
sugar_include(AkMath)
sugar_include(AkRender)
sugar_include(AkSound)

# ################ #
# # Source Files # #
# ################ #

sugar_files(AK_SOURCE 
	main.cpp 
	game.cpp 
	imp.cpp
)
