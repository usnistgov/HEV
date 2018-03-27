/**
 * \file iris.js simple Javascript interface to the IRIS Control FIFO.
 * @see doc/iris-node.7
 */

"use strict";
var fs = require("fs"),
    fmt = require("util").format;

/**
 * \brief Trim whitespace from the ends of \a s and condense multiple adjacent
 * whitespace characters into a single " " within \a s.
 * @param[in] s
 * @return the cleaned string
 */
function clean(s) {
    return s.trim().replace(/\s+/g, " ");
}

/**
 * \brief Determines if \a n is a number.
 * @param[in] n
 * @return true if \a n is a number, false otherwise.
 */
function isNumber(n) {
    return !isNan(parseFloat(n)) && isFinite(n);
}

/**
 * \brief Write \a message to \a fd synchronously.
 * @param[in] fd an open file descriptor.
 * @param[in] message a string to write. A newline character will be appended.
 * @return true if the write succeeded, false otherwise.
 */
function writeSync(fd, message) {
    var buffer = new Buffer(message + "\n");
    var written = fs.writeSync(fd, buffer, 0, buffer.length);
    if (written != buffer.length) return false;
    return true;
}

/**
 * \brief Read from \a fd synchronously, returning the result.
 * @param[in] fd an open file descriptor.
 * @param[in] bufSize the size of the buffer to use (default: 1024).
 * @return the string read, or the empty string on error.
 */
function readSync(fd, bufSize) {
    if (typeof(bufSize) === "undefined") bufSize = 1024;
    var buffer = new Buffer(bufSize);
    var read = fs.readSync(fd, buffer, 0, buffer.length);
    if (read <= 0) return "";
    return buffer.toString("utf8", 0, read).trim();
}

function createFIFO(path) {
    function _create() {
        try {
            require("mkfifo").mkfifoSync(path, 0x1FF); // 0x1FF is 777 in octal
        } catch (e) {
            console.error(fmt("%s", e));
            return false;
        }
        return true;
    }

    if (fs.existsSync(path)) {
        var st = fs.statSync(path);
        if (!st.isFIFO()) {
            fs.unlinkSync(path);
            return _create;
        }
        else return true;
    } else return _create();
}

/**
 * \brief Initialize the control and query response FIFOs asynchronously
 * @param[in] query the fifo name that query responses are returned on
 * @param[in] notify the fifo name that client notify messages are read from
 * @param[in] control the control fifo name
 * @return a structure containing the query and control FIFOs.
 */
function initFIFOs(query, notify, control) {
    if (typeof(control) === "undefined") control = process.env.IRIS_CONTROL_FIFO;

    var fifos = {
        queryName: query, query: undefined,
        notifyName: notify, notifyReadable: undefined,
        controlName: control, control: undefined,
        cleanup: function (message) {
            if (message) console.error(message);

            if (fifos.control) fs.closeSync(fifos.control);
            if (fifos.notifyReadble) fifos.notifyReadable.close();
            if (fifos.query) fs.closeSync(fifos.query);
            if (fs.existsSync(fifos.notifyName)) fs.unlinkSync(fifos.notifyName);
            if (fs.existsSync(fifos.queryName)) fs.unlinkSync(fifos.queryName);

            // Abort kills the IO threads if the FIFOs haven't been opened and
            // also ensures that notifyReadable is closed and unlinked.
            process.abort();
        }
    };

    // create the query response fifo
    if (!createFIFO(fifos.queryName)) {
        fifos.cleanup(fmt("cannot create query fifo '%s'", fifos.queryName));
    }

    // create the client notify fifo
    if (typeof(notify) !== "undefined") {
        if (!createFIFO(fifos.notifyName)) {
            fifos.cleanup(fmt("cannot create notify fifo '%s'", fifos.notifyName));
        }
    }

    // open the query fifo. this call blocks until the query fifo is opened
    // for writing by another process. the function (error, fd) will be called
    // by node.js once the fifo has been opened.
    fs.open(fifos.queryName, "r", function (error, fd) {
        fifos.query = fd;
        if (error) fifos.cleanup(fmt("cannot open query fifo '%s'", fifos.queryName));

        // read the ping response. since this callback will only be called
        // once the query fifo has been opened by another process for writing,
        // then the control fifo open callback (see below) has ideally
        // finished executing and thus the ping message has been written.
        // FIXME: there is a race condition here since node.js uses threads to
        // handle the asynchronous io callbacks.
        if (readSync(fifos.query) !== "PING iris-console") {
            fifos.cleanup(fmt("error reading query fifo '%s': invalid PING response",
                          fifos.queryName));
        }
    });

    // open the control fifo. this call blocks until the control fifo is
    // opened for reading by another process. the function (error, fd) will be
    // called by node.js once the fifo has been opened.
    fs.open(fifos.controlName, "a", function (error, fd) {
        fifos.control = fd;
        if (error) fifos.cleanup(fmt("cannot open control fifo '%s'", fifos.controlName));

        // write a ping message
        if (!writeSync(fifos.control, fmt("QUERY %s PING iris-console", fifos.queryName))) {
            fifos.cleanup(fmt("error writing control fifo '%s'", fifos.controlName));
        }
    });

    return fifos;
}

/**
 * Process a command by writing it to the control fifo and reading the
 * response if necessary.
 * @param[in] fifos the fifos "object" to use
 * @param[in] command the control command to process
 * @return the response of the command, if available, or an error message.
 */
function processCommand(fifos, command) {
    // the list of commands that will return responses
    var responseCommands = ["query"];

    var split = clean(command).split(" ");
    var index = responseCommands.indexOf(split[0].toLowerCase());
    if (index >= 0) split.splice(1, 0, fifos.queryName);

    try {
        if (!writeSync(fifos.control, split.join(" "))) {
            return fmt("error writing control fifo '%s'", fifos.controlName);
        }
    } catch (e) {
        return fmt("error writing control fifo '%s'", fifos.controlName);
    }

    try {
        if (index >= 0) return readSync(fifos.query);
    } catch (e) {
        return fmt("error reading query fifo '%s'", fifos.queryName);
    }

    return "";
}

/**
 * \brief Create a Read-Eval-Print Loop (REPL) that interacts with IRIS.
 */
function _console() {
    var fifos = initFIFOs(fmt("/tmp/irisConsoleQueryFifo-%s", process.env.USERNAME));

    // "gracefully" report uncaught exceptions
    process.on("uncaughtException", function (error) {
        console.error("uncaught exception: %s", error);
        server.fifos.cleanup();
        process.abort();
    });

    console.info("Interactive IRIS Console.\n" +
                 "Type '?' or 'help' for help.\n" +
                 "Type 'Ctrl-D' or '.exit' to quit.");
    var repl = require("repl").start({
        prompt: "iris> ",
        input: process.stdin,
        output: process.stdout,
        ignoreUndefined: true,
        eval: function (cmd, context, filename, callback) {
            // repl expects cmd to be eval'ed so it passes it in ()s
            cmd = clean(cmd.replace(/^\(/, "").replace(/\)$/, ""));
            if (typeof(cmd) === "undefined" || cmd.length == 0) return callback(null, undefined);

            if (cmd === "?" || cmd === "help") {
                console.info(fmt("Enter IRIS control commands at the prompt.\n" +
                                 "QUERY commands should NOT specify fifoName.\n" +
                                 "Command reference file://%s/doc/html/control.html",
                                 process.env.IRIS_PREFIX));
                return callback(null, undefined);
            }

            if (typeof(fifos.control) === "undefined") return callback(null, "control fifo not open");
            return callback(null, processCommand(fifos, cmd));
        }
    }).on("exit", function () {
        fifos.cleanup();
    });
}

/**
 * \brief Create a web server that interacts with IRIS. The server also opens
 * a WebSocket (RFC 6455) that listens for "control" messages and responds
 * with "response" messages.
 * @param[in] handler the function to handle HTTP requests.
 */
function _server(handler) {
    var server = {
        handler: typeof(handler) !== "undefined" ? handler : function (request, response) {
            var parsed = require("url").parse(request.url);
            if (parsed.pathname.length == 0 || parsed.pathname === "/") parsed.pathname = "index.html";
            else if (parsed.pathname[0] == "/") parsed.pathname = parsed.pathname.slice(1);

            fs.readFile(parsed.pathname, function (error, data) {
                if (error) {
                    response.writeHead(500);
                    response.end("Cannot read " + parsed.href);
                } else {
                    response.writeHead(200);
                    response.end(data);
                }
            });
        }
    };

    server.fifos = initFIFOs(fmt("/tmp/irisServerQueryFifo-%s", process.env.USERNAME),
                             fmt("/tmp/irisServerNotifyClientFifo-%s", process.env.USERNAME));

    // cleanup after ourself
    process.on('SIGINT', function() { server.fifos.cleanup(); });
    process.on('SIGTERM', function() { server.fifos.cleanup(); });

    // "gracefully" report uncaught exceptions
    process.on("uncaughtException", function (error) {
        console.error("uncaught exception: %s", error);
        server.fifos.cleanup();
        process.abort();
    });

    server.app = require("http").createServer(server.handler);
    server.io = require("socket.io").listen(server.app);
    server.io.set("log level", 2);

    server.io.sockets.on("connection", function (socket) {
        // create a readable on the notify fifo
        var createReadable = function(createReadable) {
            server.fifos.notifyReadable = fs.createReadStream(server.fifos.notifyName);

            // the readable has to be recreated since 'echo' from a shell
            // sends and EOF which causes node to close the fd and emit the
            // "end" event. I tried the autoClose option but it doesn't help.
            server.fifos.notifyReadable.on("end", function() {
                createReadable(createReadable);
            });

            server.fifos.notifyReadable.on("error", function (error) {
                // TODO: should this call cleanup and abort?
                log.warn(fmt("error on notifyReadable: %s", error));
            });

            server.fifos.notifyReadable.on("data", function (chunk) {
                socket.emit("notify", clean(chunk.toString("utf-8", 0, chunk.len)));
            });
        };

        createReadable(createReadable);

        // respond to control messages
        socket.on("control", function (cmd) {
            if (typeof(server.fifos.control) === "undefined") {
                socket.emit("response", "control fifo not open");
            } else {
                socket.emit("response", processCommand(server.fifos, clean(cmd)));
            }
        });
    });

    return server;
}

/**
 * Parse a set of options from \a argv. The \a options variable is an array
 * with the following format:
 * \code
 * [
 *   { name: "longoption", opt: "l", required: false, value: true, defaultValue: 42 },
 *   { name: "long2", opt: "L", required: true, value: false }
 * ]
 * \endcode
 * @param[in] argv the process.argv value to parse.
 * @param[in] options an array of options.
 */
function _parseArgs(argv, options) {
    var args = {};
    if (typeof(options) === "undefined") return args;

    var progname = argv[1];
    argv = argv.slice(2);

    for (var i = 0; i < argv.length; ++i) {
        var optopt = argv[i];
        if (optopt[0] === "-") {
            if (optopt[1] === "-") optopt = optopt.slice(2);
            else optopt = optopt.slice(1);

            var j;
            for (j = 0; j < options.length; ++j) {
                if (options[j].name === optopt || options[j].opt === optopt) break;
            }

            if (j == options.length) {
                console.log(fmt("%s: unrecognized option '%s'", progname, optopt));
                return undefined;
            }

            if (options[j].value) {
                if (argv.length <= i+1 || argv[i+1][0] == "-") {
                    console.log(fmt("%s: option '%s' requires a value", progname, options[j].name));
                    return undefined;
                }

                args[options[j].name] = argv[++i];
            } else {
                args[options[j].name] = true;
            }
        }
    }

    for (var i = 0; i < options.length; ++i) {
        if (!(options[i].name in args)) {
            if (options[i].required) {
                console.log(fmt("%s: option '%s' is required", progname, options[i].name));
            }
            args[options[i].name] = options[i].defaultValue;
        }
    }

    return args;
}

exports.console = _console;
exports.server = _server;
exports.parseArgs = _parseArgs;

