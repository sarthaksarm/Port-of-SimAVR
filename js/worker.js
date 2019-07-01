function print(text) {
    self.postMessage({
        cmd: 'print',
        text: text
    });
}

var Module = {
    arguments: ["-v", "--menu"],
    preRun: [],
    postRun: [],
    print: function(text) {
        print(text);
    },
    printErr: function(text) {
        print(text);
    },
    totalDependencies: 0
};

self.importScripts('simavr.js');

function port_changed(port, value) {
    self.postMessage({
        cmd: 'port_changed',
        port: port,
        value: value
    });
}

function get_ports() {
    var get_ports = Module.cwrap('get_ports', 'string')
    pstr = get_ports();
    self.postMessage({cmd: 'get_ports', pstr: pstr});
}

var timer = null;
var interval = 20;

function run_sim(name) {
    var initf =  Module.cwrap('init', 'void', ['string']);

    console.log(name);
    initf(name);


    timer = self.setInterval(function() {
        Module._run();
    }, interval);
}

function restart_timer(i) {
    interval = i;
    
    if(timer == null)
        return;    

    self.clearInterval(timer);
    timer = self.setInterval(function() {
        Module._run();
    }, interval);
}

function create_file(data) {
    dat = data;
    var dat = dat.substring(dat.indexOf(",") + 1);
    dat = atob(dat);    //performs decoding of data that was encoded to base64 

    var name = "f-" +  Math.floor(Date.now() / 1000) + ".hex";

    console.log("creating file " + name);
    FS.createDataFile("/", name, dat, true, false);
    console.log("running");
    run_sim(name);
}

function set_pin(port, pin, value) {
    Module._set_pin(port, pin, value);
}

self.addEventListener('message', function(e) {
    var d = e.data;

    if(d.cmd == 'get_ports') {
        get_ports();
    } else if(d.cmd == 'create_file') {
        create_file(d.data);
    } else if(d.cmd == 'set_pin') {
        set_pin(d.port, d.pin, d.value);
    } else if(d.cmd == 'set_speed') {
        restart_timer(d.speed);
    }
}, false);

console.log("Worker running");
