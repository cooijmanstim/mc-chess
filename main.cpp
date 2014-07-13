#include <ios>
#include <stack>

#include <boost/algorithm/string.hpp>

#include "move.hpp"
#include "state.hpp"
#include "random_agent.hpp"
#include "mcts_agent.hpp"
#include "notation.hpp"

#define fmt boost::format

class Game {
  std::stack<State> history;

public:
  State& current_state() {
    return history.top();
  }

  Color color_to_move() {
    return current_state().us;
  }

  Game() {
    history = std::stack<State>();
    history.emplace();
  }

  void make_move(Move move) {
    history.push(State(current_state()));
    current_state().make_move(move);
  }

  void undo() {
    history.pop();
  }

  void setboard(std::string fen) {
    // not sure about when setboard will be called. assuming after "new"
    // and before anything that modifies the state.
    history = std::stack<State>();
    history.emplace(fen);
  }
};

const std::vector<std::string> features = {
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
  "debug=1",
  "memory=0",
  "smp=0",
  "done=1",
};

void interface_with(std::istream& in, std::ostream& out) {
  out.setf(std::ios::unitbuf);

  unsigned time_budget = 60;

  bool debug = false;
  
  Game game;
  MCTSAgent agent(2);
  boost::optional<Color> agent_color;

  boost::future<void> future_quit;
  boost::future<Move> future_decision;
  boost::future<std::string> future_command;

  auto protocol_debug = [&](std::string s) {
    if (debug)
      out << "#" << s << std::endl;
  };

  auto recv_command = [&]() {
    std::string line;
    if (!std::getline(in, line))
      future_quit = boost::make_ready_future();
    protocol_debug("<xboard> " + line);
    return line;
  };

  auto send_command = [&](std::string line) {
    protocol_debug("<engine> " + line);
    out << line << std::endl;
  };

  auto report_illegal = [&](std::string move) {
    send_command("Illegal move: " + move);
  };

  auto report_error = [&](std::string class_, std::string instance) {
    send_command("Error (" + class_ + "): " + instance);
  };

  auto protocol_assert = [&](bool condition, std::string message) {
    if (!condition)
      report_error("protocol assumption violated", message);
  };

  typedef std::vector<std::string> ARGV;

  std::function<void(std::vector<std::string>)> do_nothing = [&](ARGV argv){};
  std::function<void(std::vector<std::string>)> unsupported = [&](ARGV argv){
    report_error("unsupported", argv[0]);
  };

  std::map<std::string, std::function<void(std::vector<std::string>)> > handlers = {
    {"xboard", do_nothing},
    {"protover", [&](ARGV argv) {
        for (auto feature: features)
          send_command("feature " + feature);
      }},
    // TODO: handle these as soon as we get a use case
    {"accepted", do_nothing},
    {"rejected", unsupported},
    {"new", [&](ARGV argv) {
        protocol_assert(!agent_color, "expected \"new\" only in force mode");
        game = Game();
        agent_color = colors::black;
        agent.set_state(game.current_state());
      }},
    {"variant", unsupported},
    {"quit", [&](ARGV argv) {
        agent.abort_decision();
        agent.stop_pondering();
        future_quit = boost::make_ready_future();
      }},
    {"random", do_nothing},
    {"force", [&](ARGV argv) {
        agent_color = boost::none;
        agent.abort_decision();
        agent.stop_pondering();
      }},
    {"go", [&](ARGV argv) {
        agent_color = game.color_to_move();
        agent.set_state(game.current_state());
        agent.start_pondering();
        future_decision = agent.start_decision(time_budget);
      }},
    {"playother", [&](ARGV argv) {
        protocol_assert(!agent_color, "expected \"playother\" only in force mode");
        agent_color = colors::opposite(game.color_to_move());
      }},
    {"level", unsupported},
    {"st", unsupported},
    {"sd", unsupported},
    {"nps", unsupported},
    {"time", unsupported},
    {"otim", unsupported},
    {"usermove", [&](ARGV argv) {
        protocol_assert(!agent_color || agent_color != game.color_to_move(),
                        "usermove during engine's turn");
        Move move;
        try {
          move = notation::algebraic::parse(argv[1], game.current_state());
        } catch (notation::algebraic::OverdeterminedException& e) {
          report_illegal(argv[1]);
          return;
        } catch (notation::algebraic::UnderdeterminedException& e) {
          report_error("ambiguous move", argv[1]);
          return;
        }
        game.make_move(move);
        if (agent_color) {
          agent.advance_state(move);
          agent.start_pondering();
          future_decision = agent.start_decision(time_budget);
        }
      }},
    {"?", [&](ARGV argv) {
        agent.finalize_decision();
      }},
    {"ping", [&](ARGV argv) {
        if (future_decision.valid())
          future_decision.wait();
        send_command("pong " + argv[1]);
      }},
    {"draw", [&](ARGV argv) {
        protocol_assert(agent_color, "\"draw\" when engine not assigned to any color");
        agent.set_state(game.current_state());
        if (agent.accept_draw(*agent_color))
          send_command("offer draw");
      }},
    {"result", [&](ARGV argv) {
        agent.idle();
      }},
    {"setboard", [&](ARGV argv) {
        protocol_assert(!agent_color, "expected \"setboard\" only in force mode");
        std::string fen = boost::algorithm::join(ARGV(argv.begin() + 1, argv.end()), " ");
        game.setboard(fen);
        agent.set_state(game.current_state());
      }},
    {"edit", unsupported},
    {"hint", unsupported},
    {"bk", do_nothing},
    {"undo", [&](ARGV argv) {
        protocol_assert(!agent_color, "expected \"undo\" only in force mode");
        game.undo();
        agent.set_state(game.current_state());
      }},
    {"remove", [&](ARGV argv) {
        protocol_assert(!agent_color || agent_color != game.color_to_move(),
                        "expected \"remove\" only during user's turn");
        game.undo();
        game.undo();
        agent.set_state(game.current_state());
      }},
    {"hard", do_nothing},
    {"easy", do_nothing},
    {"post", do_nothing},
    {"nopost", do_nothing},
    {"analyze", unsupported},
    {"name", do_nothing},
    {"rating", do_nothing},
    {"ics", unsupported},
    {"computer", do_nothing},
    {"pause", [&](ARGV argv) {
        agent.pause();
      }},
    {"resume", [&](ARGV argv) {
        agent.resume();
      }},
    // this could prove to be useful
    {"memory", unsupported},
    {"cores", unsupported},
    {"egtpath", do_nothing},
    {"option", do_nothing},
  };

  auto handle_decision = [&](){
    Move move = future_decision.get();
    send_command("move " + notation::coordinate::format(move));
    game.make_move(move);
    agent.advance_state(move);
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
    future_decision = boost::async([]() -> Move {
        sleep_forever();
        throw std::runtime_error("greetings, time traveler!");
      });
  };
  
  auto handle_command = [&]() {
    std::string line = future_command.get();
    std::vector<std::string> argv;
    boost::algorithm::split(argv, line, boost::algorithm::is_any_of("\t "), boost::token_compress_on);
    std::function<void(ARGV)> handler;
    try {
      handler = handlers.at(argv[0]);
    } catch (std::out_of_range& e) {
      protocol_assert(false, "unknown command: " + line);
      goto after_handler;
    }
    handler(argv);

    after_handler:
    future_command = boost::async(recv_command);
  };
  
  future_command = boost::async(recv_command);
  while (true) {
    unsigned short ifuture;
    try {
      ifuture = boost::wait_for_any(future_quit, future_decision, future_command);
    } catch (boost::thread_interrupted&) {
    }

    switch (ifuture) {
    case 0:
      goto quit;
      break;
    case 1:
      handle_decision();
      break;
    case 2:
      handle_command();
      break;
    }

    ifuture = -1;
  }

  quit:
  ;
}

int main(int argc, char* argv[]) {
  interface_with(std::cin, std::cout);
}
