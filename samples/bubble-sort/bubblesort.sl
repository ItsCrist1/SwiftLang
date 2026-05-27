$v[] <- input.in

for($i <- 0; $i < $v[]; $i <- $i + 1) {
    for($j <- 0; $j < $v[]-$i-1; $j <- $j + 1) {
        if($v[$j] > $v[$j+1]) {
            $t <- $v[$j]
            $v[$j] <- $v[$j+1]
            $v[$j+1] <- $t
        }
    }
}

for($i <- 0; $i < $v[]; $i <- $i + 1) {
    cp $v[$i]
}
