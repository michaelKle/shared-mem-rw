var SharedMemory = require("./")

console.log(SharedMemory);
var a = new SharedMemory.SharedMemory('test', "CREATE_RW", 10)
console.log("name:    " + a.name)
console.log("length:  " + a.length)

a.set(0, 0);
a.set(1, 1);
a.set(2, 2);
buf = new Uint8Array(a.mem());
console.log(`buf=${buf[0]} ${buf[1]} ${buf[2]} ${buf[3]}`)
a.close()

console.log("name:    " + a.name)
console.log("length:  " + a.length)
