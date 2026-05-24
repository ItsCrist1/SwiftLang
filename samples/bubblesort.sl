$v[] <- input.in

$i <- 0

while($i < $v[]) {
    $j <- 0

    while($j < $v[]-$i-1) {
        if($v[$j] > $v[$j+1]) {
            $t <- $v[$j]
            $v[$j] <- $v[$j+1]
            $v[$j+1] <- $t
        }

        $j <- $j + 1
    }

    $i <- $i + 1
}

$i <- 0

while($i < $v[]) {
    cp $v[$i]
    $i <- $i + 1
}