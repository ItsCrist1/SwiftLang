$n <- input.in

if ($n < 2) {
    cp NP
    ce
}

if ($n < 4) {
    cp P
    ce
}

if ($n % 2 == 0 || $n % 3 == 0) {
    cp NP
    ce
}

for ($i <- 5; $i*$i <= $n; $i <- $i + 6) {
    if ($n % $i == 0 || $n % ($i+2) == 0) {
        cp NP
        ce
    }
}

cp P