$x <- input.in

while($x % 2 == 0) {
    cp "2"
    $x <- $x / 2
}

for($i <- 3; $i*$i <= $x; $i <- $i + 2) {
    while($x % $i == 0) {
        cp $i
        $x <- $x / $i
    }
}

if($x > 2) {
    cp $x
}