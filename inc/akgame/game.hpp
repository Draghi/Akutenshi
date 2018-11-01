#ifndef AK_GAME_GAME_HPP_
#define AK_GAME_GAME_HPP_

int akGameMain();

namespace akg {

	void startup(const akl::Logger& log);

	void cleanup(const akl::Logger& log);

	void runGame();

}

#endif
