$x < input.in

while[$x % 2 == 0]
    eh "2"
    $x < $x / 2
}

$i < 3

while[$i*$i <= $x]
    while[$x % $i == 0]
        eh $i
        $x < $x / $i
    }

    $i < $i + 2
}

if[$x _> 2]
    eh $x
}