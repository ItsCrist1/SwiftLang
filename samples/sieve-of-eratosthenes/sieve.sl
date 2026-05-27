$sz <- input.in

# Convention
$v[0] <- 0
$v[1] <- 0

# Set every bit to 1
for($i <- 2; $i < $sz; $i <- $i + 1) {
    $v[$i] <- 1
}

# Rule out even numbers
for($i <- 4; $i < $sz; $i <- $i + 2) {
    $v[$i] <- 0
}

$i <- 3
$si <- 9

# The algorithm itself
while($si < $sz) {
    if($v[$i]) {
        for($j <- $si; $j < $sz; $j <- $j + $i*2) {
            $v[$j] <- 0
        }
    }

    $i <- $i + 2
    $si <- $i * $i
}

# Output
for($i <- 0; $i < $sz; $i <- $i + 1) {
    cp $i " -> " $v[$i]
}
