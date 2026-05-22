$n < input.in

if($n <_ 2) {
    cp NP;
    ce
}

if($n <_ 4) {
    cp P;
    ce
}

if($n % 2 == 0 || $n % 3 == 0) {
    cp NP;
    ce
}

$i < 5

while($i*$i <= $n) {
    if($n % $i == 0 || $n % ($i+2) == 0) {
        cp NP;
        ce
    }

    $i < $i + 6
}

cp P