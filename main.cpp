#include <ios>
#include <thread>
#include <chrono>

const auto features = {
  "done=0",
  "ping=1",
  "setboard=1",
  "playother=1",
  "san=1",
  "usermove=1",
  "time=0",
  "draw=1",
  "sigint=0",
  "sigterm=0",
  "reuse=1",
  "analyze=0",
  "myname=\"mc-chess\"", // TODO: version
  "feature=\"normal\"",
  "colors=0",
  "ics=0",
  "name=0",
  "pause=1",
  "nps=0",
  "debug=0",
  "memory=0",
  "smp=0",
  "done=1",
};

std::string read_command() {
  return std::getline(std::cin, line);
}

int main(int argc, char* argv[]) {
  std::cout.setf(ios::unitbuf);
  
#define protocol_assert(condition, message) \
  if (!(condition)) \
    tellError("protocol assumption violated", message);
#define tell(s) std::cout << s << std::endl;
#define tellIllegal(s) tell("Illegal move: " << s);
#define tellError(s, t) tell("Error (" << s << "): " << t);
  
  State state;
  Agent agent;
  boost::optional<Color> agent_color;

  // for undo
  std::stack<State> history;

  std::function<void(std::vector<std::string>)> do_nothing = [](auto argv){};
  std::function<void(std::vector<std::string>)> unsupported = [](auto argv){
    tell("telluser Unsupported"); // FIXME: or something like this
  };

  std::future<void> future_quit;
  std::future<Move> future_decision;
  std::future<std::string> future_command;

  std::map<std::string, std::function<void(std::vector<std::string>)> > handlers = {
    "xboard": do_nothing,
    "protover": [](auto argv) {
      for (auto feature: features)
        tell("feature " << feature);
    },
    // TODO: handle these as soon as we get a use case
    "accepted": unsupported,
    "rejected": unsupported,
    "new": [&state, &agent, &agent_color, &history](auto argv){
      protocol_assert(!agent_color, "expected \"new\" only in force mode");
      state.set_initial_configuration();
      history = {};
      agent_color = colors::black;
    },
    "variant": unsupported,
    "quit": [&agent, &future_quit](auto argv){
      agent.abort_decision();
      agent.stop_pondering();
      future_quit = boost::make_future();
    },
    "random": do_nothing,
    "force": [&agent, &agent_color](auto argv) {
      agent_color = boost::none;
      agent.abort_decision();
      agent.stop_pondering();
    },
    "go": [&state, &agent, &agent_color, &future_decision](auto argv) {
      agent_color = state.color_to_move();
      agent.start_pondering(state);
      future_decision = agent.start_decision(state);
    },
    "playother": [&state, &agent, &agent_color](auto argv) {
      protocol_assert(!agent_color, "expected \"playother\" only in force mode");
      agent_color = colors::opposite(state.color_to_move());
    },
    "level": unsupported,
    "st": unsupported,
    "sd": unsupported,
    "nps": unsupported,
    "time": unsupported,
    "otim": unsupported,
    "usermove": [&state, &agent, &agent_color, &history](auto argv) {
      protocol_assert(!agent_color || agent_color != state.color_to_move(),
                      "usermove during engine's turn");
      try {
        Move move = state.parse_algebraic(argv[1]);
      } catch (AlgebraicOverdeterminedException& e) {
        tellIllegal(argv[1]);
        return;
      } catch (AlgebraicUnderdeterminedException& e) {
        tellError("ambiguous move", argv[1]);
        return;
      }
      history.push(state);
      state.make_move(move);
      if (agent_color) {
        agent.start_pondering(state);
        future_decision = agent.start_decision(state);
      }
    },
    "?": [&agent](auto argv) {
      agent.finalize_decision();
    },
    "ping": [&future_decision](auto argv) {
      if (future_decision.valid())
        future_decision.wait();
      tell("pong " << argv[0]);
    },
    "draw": [&state, &agent](auto argv) {
      if (agent.accept_draw(state, agent_color))
        tell("offer draw");
    },
    "result": [&agent](auto argv) {
      agent.idle();
    },
    "setboard": [&state, &agent](auto argv) {
      protocol_assert(!agent_color, "expected \"setboard\" only in force mode");
      std::string fen = boost::algorithm::join(argv[1:], " ")
      state = new State(fen);
    },
    "edit": unsupported,
    "hint": unsupported,
    "bk": do_nothing,
    "undo": [&state, &agent, &history, &agent_color](auto argv) {
      protocol_assert(!agent_color, "expected \"undo\" only in force mode");
      state = history.pop();
    },
    "remove": [&state, &agent, &history, &agent_color](auto argv) {
      protocol_assert(!agent_color || agent_color != state.color_to_move(),
                      "expected \"remove\" only during user's turn");
      history.pop();
      state = history.pop();
    },
    "hard": do_nothing,
    "easy": do_nothing,
    "post": do_nothing,
    "nopost": do_nothing,
    "analyze": unsupported,
    "name": do_nothing,
    "rating": do_nothing,
    "ics": unsupported,
    "computer": do_nothing,
    "pause": [&agent](auto argv) {
      agent.pause();
    },
    "resume": [&game](auto argv) {
      agent.resume();
    },
    // this could prove to be useful
    "memory": unsupported,
    "cores": unsupported,
    "egtpath": do_nothing,
    "option": do_nothing,
  };

  future_command = std::async(read_command);
  while (true) {
    unsigned short ifuture;
    while (true) {
      try {
        ifuture = boost::wait_for_any(future_quit, future_decision, future_command);
      } catch (boost::thread_interrupted&) {
      }
    }

    switch (ifuture) {
    case 0:
      goto quit;
      break;
    case 1:
      tell("move " << future_decision.get().to_can());
      // there has to be a better way to make sure we don't keep redetecting
      // the same future decision value, but i don't know it.  really, the
      // wait_for_any function should wait for any one of them to turn from
      // not-yet-ready to ready, but the manual says that's not how it works.
      // so, replace the ready future with one that will never be ready.
      // i considered keeping a set of futures that need waiting for, and then
      // simply removing processed futures.  but the futures have different
      // value types so we can't store them in a single container unless we use
      // something like boost::variant and then we need a boost::visitor and a
      // boost::welcome_committee and a boost::mayor to ceremonially snip a
      // ribbon with a giant pair of scissors and...  what was the question?
      future_decision = std::async([&state]() {
          while (true)
            boost::this_thread::sleep_for(std::chronos::years(100));
          throw std::runtime_error("greetings, time traveler!");
      });
      break;
    case 2:
      std::string line = future_command.get();
      std::vector<std::string> argv;
      boost::algorithm::split(argv, line, boost::algorithm::is_any_of("\t "), boost::token_compress_on);
      try {
        auto handler = handlers.at(argv[0]);
      } catch (std::out_of_range& e) {
        protocol_assert(false, "unknown command: " << line);
        continue;
      }
      handler(argv);

      future_command = std::async(read_command);
      break;
    }
  }

  quit:
}
