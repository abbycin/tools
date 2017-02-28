use std::thread;
use std::env::args;
use std::sync::mpsc::{channel, Receiver, Sender};
use std::time::Instant;

fn sender(tx: Sender<u64>, data: u64, mut limit: u64) {
    loop {
        if limit == 0 { break; }
        limit -= 1;
        tx.send(data).unwrap();
    }
}

fn receiver(rx: Receiver<u64>) {
    let mut counter = vec![0, 0, 0];
    loop {
        match rx.recv().unwrap() {
            0 => counter[0] += 1,
            1 => counter[1] += 1,
            2 => counter[2] += 1,
            _ => break
        }
    }
    for (idx, data) in counter.iter().enumerate() {
        println!("thread {} => {}", idx, data);
    }
    println!("receiver done.");
}

fn main() {
    if args().len() < 2 {
        println!("{} num", args().nth(0).unwrap());
        std::process::exit(1);
    }
    let num: u64 = args().nth(1).unwrap().parse().unwrap();
    let (tx, rx) = channel();
    let (tx1, tx2, tx3) = (tx.clone(), tx.clone(), tx.clone());
    let now = Instant::now();
    let rcv = thread::spawn(move || {
        receiver(rx);
    });
    let tid1 = thread::spawn(move || {
        sender(tx1, 0, num);
        println!("thread 0 done");
    });
    let tid2 = thread::spawn(move || {
        sender(tx2, 1, num);
        println!("thread 1 done");
    });
    let tid3 = thread::spawn(move || {
        sender(tx3, 2, num);
        println!("thread 2 done");
    });
    tid1.join().unwrap();
    tid2.join().unwrap();
    tid3.join().unwrap();
    tx.send(309).unwrap();
    rcv.join().unwrap();
    let dur = now.elapsed();
    println!("{}.{}", dur.as_secs(), dur.subsec_nanos());
}
