var avrsim = {
    timer: false,
    leds: [],
    btns: [],
    ports: [],
    worker: null,
    get_ports: function() {
        this.worker.postMessage({cmd: 'get_ports'});
    },
    load_ports: function(pstr) {
        console.log("Registering ports " + pstr);

        for(var i = 0, len = pstr.length; i < len; i++) {
            this.ports.push(pstr[i]);
        }
        
        var self = this;
        this.ports.forEach(function(val) {
            var lobj = [];
            var bobj = [];

            for(var i = 7; i >= 0; i--) {
                lobj.push("P" + val + i);
                bobj.push("P" + val + i);
            }

            self.leds.push(lobj);
            self.btns.push(bobj);
        });

        this.render();
    },
    init_ports: function() {
        this.get_ports();
    },
    print: function(text) {
        $(".mcu-terminal").append("<li>" + text + "</li>");
        console.log(text);
    },
    worker_cb: function(e, self) {
        var d = e.data;
        
        if(d.cmd == 'port_changed') {
            self.port_changed(d.port, d.value);
        } else if(d.cmd == 'get_ports') {
            self.load_ports(d.pstr);
        } else if(d.cmd == 'print') {
            self.print(d.text);
        }
    },
    init_worker: function() {
        this.print("Initializing worker...");
        this.worker = new Worker('js/worker.js');

        var self = this;            //here this is pointing to avrsim
        // console.log("in the init worker")
        // console.log(this == avrsim)
        this.worker.addEventListener('message', function(e) {
            self.worker_cb(e, self);
        }.bind(self), false);
        this.worker.postMessage({cmd: 'init'});
    },
    set_speed: function(speed) {
        this.worker.postMessage({
            cmd: 'set_speed',
            speed: speed
        });
    },
    init_slider: function() {
        var self = this;

        $('#speed').on('change', function() {
            var s = 101 - $('#speed').val();
            self.set_speed(s);
        }.bind(self));
    },
    init: function() {
        nunjucks.configure({autoescape: true});         //todo : watch a video on this

        this.init_worker();
        this.init_ports();
        this.init_slider();
    },
    create_file: function(data) {
        this.worker.postMessage({cmd: 'create_file', data: data});
    },
    handle_load: function() {
        if(this.timer != false)
            window.clearInterval(this.timer);
    
        this.timer = false;

        var files = $("#file-sel").get(0).files;
        console.log(files)
        var self = this;
        var output = [];
        for (var i = 0, file; file = files[i]; i++) {
            var reader = new FileReader();

            reader.onload = function(event) {
                var data = event.target.result;
                self.create_file(data);
            };

            reader.readAsDataURL(file);
        }
    },
    set_pin: function(port, pin, value) {
        this.worker.postMessage({cmd: 'set_pin', port: port, pin: pin, value: value});
    },
    register_btn_handlers: function() {
        var self = this;

        this.btns.forEach(function(val) {
            val.forEach(function(ival) {
                var elem = $("#btn-" + ival);
                elem.click(function() {

                    var port = ival.substring(1, 2);
                    var pin = ival.substring(2, 3);
                    var val;

                    if(!elem.hasClass("darken-4")) {
                        val = 1;
                        elem.addClass("darken-4");
                    } else {
                        val = 0;
                        elem.removeClass("darken-4");
                    }
                    
                    self.set_pin(port.charCodeAt(0), pin, val);
                });
            });
        });
    },
    render: function() {
        var self = this;
        nunjucks.render("tpl/leds.html", { leds: this.leds }, function(err, res) {
            console.log(err);
            $('#leds').append(res);
        });

        nunjucks.render("tpl/btns.html", { btns: this.btns }, function(err, res) {
            console.log(err);
            $('#btns').append(res);
            self.register_btn_handlers();
        });
    },
    port_changed: function(port, val) {
        for(var pin = 0; pin < 8; pin++) {
            var id = "led-P" + String.fromCharCode(port) + pin;
            var e = document.getElementById(id);

            // this is faster than addClass and removeClass, but not as flexible
            // because the color is hardcoded
            if(val & (1<<pin)) {
                e.setAttribute('style', 'background-color: #F44336 !important');
            } else {
                e.setAttribute('style', 'background-color: #9e9e9e !important');
            }
        }
    }
};

$(function(){
    $(".modal-trigger").click(function(){
        $("#agree").click(function(){
            avrsim.handle_load();
        })
    });

    var self = avrsim;
        $('#abort').click(function() {
            console.log("signaling stop...");
            self.worker.postMessage({cmd: 'stop'});
        });
    avrsim.init();
})