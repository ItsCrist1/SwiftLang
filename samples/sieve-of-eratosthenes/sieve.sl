$sz <- input.in
$i <- 2

# Convention
$v[0] <- 0
$v[1] <- 0

# Set every bit to 1
while($i < $sz) {
    $v[$i] <- 1
    $i <- $i + 1
}

# Rule out even numbers
$i <- 4

while($i < $sz) {
    $v[$i] <- 0
    $i <- $i + 2
}

$i <- 3
$si <- 9

# The algorithm itself
while($si < $sz) {
    if($v[$i]) {
        $j <- $si

        while($j < $sz) {
            $v[$j] <- 0
            $j <- $j + $i*2
        }
    }

    $i <- $i + 2
    $si <- $i * $i
}

# Output
$i <- 0
while($i < $sz) {
    cp $i " -> " $v[$i]

    $i <- $i + 1
}