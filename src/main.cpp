#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/String.hpp>
#include <akengine/debug/Log.hpp>
#include <bits/exception.h>
#include <process.h>
#include <stdlib.h>
#include <unistd.h>
#include <csignal>
#include <exception>
#include <string>
#include <typeinfo>

[[noreturn]] static void termHandler();

extern int akGameMain();

int main(int /*argc*/, char* /*argv*/[]) {
	std::set_terminate(termHandler);
	return akGameMain();
}

// ////////////////////////// //
// // Unhandled Exceptions // //
// ////////////////////////// //

#ifdef __GNUC__
#include <cxxabi.h>
#endif

[[noreturn]] static void termHandler() {
	if( auto exc = std::current_exception() ) {
		try {
			rethrow_exception(exc);
		} catch (const std::exception& e) {

		std::string rawName = typeid(e).name();
		std::string whatString = e.what();
		whatString = aku::buildString("'. Message: ", whatString.empty() ? "<Empty>" : "\n", whatString);

#		ifdef __GNUC__
			int status = 0;
			char* typeName = abi::__cxa_demangle(rawName.data(), nullptr, nullptr, &status);
			if (typeName == nullptr) typeName = rawName.data();
			akl::Logger("Term").fatal("Terminate called after throwing a '", typeName, whatString);
			if (typeName != rawName.data()) free(typeName);
#		else
			akl::Logger("Term").fatal("Terminate called after throwing a '", rawName, whatString);
#		endif

		} catch (const uint8& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'uint8' with the value: ", static_cast<uint64>(e));
		} catch (const uint16& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'uint16' with the value: ", static_cast<uint64>(e));
		} catch (const uint32& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'uint32' with the value: ", static_cast<uint64>(e));
		} catch (const uint64& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'uint64' with the value: ", static_cast<uint64>(e));
		} catch (const int8& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'int8' with the value: ", static_cast<int64>(e));
		} catch (const int16& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'int16' with the value: ", static_cast<int64>(e));
		} catch (const int32& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'int32' with the value: ", static_cast<int64>(e));
		} catch (const int64& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'int64' with the value: ", static_cast<int64>(e));
		} catch (const std::string& e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'std::string' with the value: ", e);
		} catch (void* e) {
			akl::Logger("Term").fatal("Terminate called after throwing a 'pointer' with the value: ", reinterpret_cast<intptr_t>(e));
		} catch(...) {
			akl::Logger("Term").fatal("Terminate called after throwing an unknown type.");
		}
	} else {
		akl::Logger("Term").warn("Terminate called.");
	}

	akl::processMessageQueue();
	akl::processMessageQueue();

	std::abort();
}

// ///////////////////////////// //
// //    Linux  Stacktrace    // //
// ///////////////////////////// //
// // Run while you still can // //
// ///////////////////////////// //

#if defined(__linux)
#include <features.h>
#include <ucontext.h>
#define BACKWARD_HAS_BFD 1
#include "backward.hpp"

#ifdef REG_RIP // x86_64
#define BW_GET_ERROR_ADDRESS(x) reinterpret_cast<void*>(x->uc_mcontext.gregs[REG_RIP])
#elif defined(REG_EIP) // x86_32
#define BW_GET_ERROR_ADDRESS(x) reinterpret_cast<void*>(x->uc_mcontext.gregs[REG_EIP])
#elif defined(__arm__)
#define BW_GET_ERROR_ADDRESS(x) reinterpret_cast<void*>(x->uc_mcontext.arm_pc)
#elif defined(__aarch64__)
#define BW_GET_ERROR_ADDRESS(x) reinterpret_cast<void*>(x->uc_mcontext.pc);
#elif defined(__ppc__) || defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__)
#define BW_GET_ERROR_ADDRESS(x) reinterpret_cast<void*>(x->uc_mcontext.regs->nip)
#else
#	warning ":/ sorry, ain't know no nothing none not of your architecture!"
#endif

#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
# define BW_PRINT_SIGINFO(x, destStream)           \
{                                                  \
	int saved_stderr = dup(STDERR_FILENO);         \
	int out_pipe[2];                               \
	if(pipe(out_pipe) == 0 ) {                     \
		dup2(out_pipe[1], STDERR_FILENO);          \
		close(out_pipe[1]);                        \
	                                               \
		long flags = fcntl(out_pipe[0], F_GETFL);  \
		flags |= O_NONBLOCK;                       \
		fcntl(out_pipe[0], F_SETFL, flags);        \
	                                               \
		psiginfo(x, nullptr);                      \
		fflush(stderr);                            \
	                                               \
		char buffer[8096] = {0};                   \
		read(out_pipe[0], buffer, 8096 - 1);       \
	                                               \
		dup2(saved_stderr, STDERR_FILENO);         \
	                                               \
	    destStream << buffer;                      \
	}                                              \
}
#else
# define BW_PRINT_SIGINFO(x, destStream)
#endif

namespace backward {

	// Copied from backward source and modified to print to log.
	class CustomSignalHandling {
		public:
		   static std::vector<int> make_default_signals() {
			   const int posix_signals[] = {
				// Signals for which the default action is "Core".
				SIGABRT,    // Abort signal from abort(3)
				SIGBUS,     // Bus error (bad memory access)
				SIGFPE,     // Floating point exception
				SIGILL,     // Illegal Instruction
				SIGIOT,     // IOT trap. A synonym for SIGABRT
				SIGQUIT,    // Quit from keyboard
				SIGSEGV,    // Invalid memory reference
				SIGSYS,     // Bad argument to routine (SVr4)
				SIGTRAP,    // Trace/breakpoint trap
				SIGXCPU,    // CPU time limit exceeded (4.2BSD)
				SIGXFSZ,    // File size limit exceeded (4.2BSD)
			};
				return std::vector<int>(posix_signals, posix_signals + sizeof posix_signals / sizeof posix_signals[0] );
		   }

		   CustomSignalHandling(const std::vector<int>& posix_signals = make_default_signals()): _loaded(false) {
				bool success = true;

				const size_t stack_size = 1024 * 1024 * 8;
				_stack_content.reset(static_cast<char*>(malloc(stack_size)));
				if (_stack_content) {
					stack_t ss;
					ss.ss_sp = _stack_content.get();
					ss.ss_size = stack_size;
					ss.ss_flags = 0;
					if (sigaltstack(&ss, nullptr) < 0) success = false;
				} else {
					success = false;
				}

				for (size_t i = 0; i < posix_signals.size(); ++i) {
					struct sigaction action;
					memset(&action, 0, sizeof action);
					action.sa_flags = static_cast<int>(SA_SIGINFO | SA_ONSTACK | SA_NODEFER | SA_RESETHAND);
					sigfillset(&action.sa_mask);
					sigdelset(&action.sa_mask, posix_signals[i]);
					action.sa_sigaction = &sig_handler;

					int r = sigaction(posix_signals[i], &action, nullptr);
					if (r < 0) success = false;
				}

				_loaded = success;
			}

			bool loaded() const { return _loaded; }

			static void handleSignal(int, siginfo_t* info, void* _ctx) {
				ucontext_t *uctx = static_cast<ucontext_t*>(_ctx);

				StackTrace st;
				void* error_addr = BW_GET_ERROR_ADDRESS(uctx);

				if (error_addr) {
					st.load_from(error_addr, 32);
				} else {
					st.load_here(32);
				}

				std::stringstream sstream;

				Printer printer;
				printer.address = true;
				printer.print(st, sstream);

				BW_PRINT_SIGINFO(info, sstream)

				akl::Logger("Signal").fatal("Received fatal signal, program terminating.\n", sstream.str());

				akl::processMessageQueue();
				akl::processMessageQueue();
			}

		private:
			details::handle<char*> _stack_content;
			bool                   _loaded;

			[[noreturn]] static void sig_handler(int signo, siginfo_t* info, void* _ctx) {
				handleSignal(signo, info, _ctx);
				raise(info->si_signo);
				puts("watf? exit");
				_exit(EXIT_FAILURE);
			}
	};

	static backward::CustomSignalHandling sh;
}

#endif


