$x < input.in

while ($x % 2 == 0) {
    cp "2"
    $x < $x / 2
}

$i < 3

while ($i*$i <= $x) {
    while ($x % $i == 0) {
        cp $i
        $x < $x / $i
    }

    $i < $i + 2
}

if ($x _> 2) {
    cp $x
}