$v[] <- input.in
$doSort <- do-sort.in

# Sorting
if($doSort) {
    for($i <- 0; $i < $v[]; $i <- $i + 1) {
        for($j <- 0; $j < $v[]-$i-1; $j <- $j + 1) {
            if($v[$j] > $v[$j+1]) {
                $t <- $v[$j]
                $v[$j] <- $v[$j+1]
                $v[$j+1] <- $t
            }
        }
    }
}
# Binary Search
$t <- target.in
$lo <- 0
$hi <- $v[]-1
$found <- 0
$foundFlag <- 0

while($lo <= $hi) {
    $mid <- ($lo+$hi) / 2
    if($v[$mid] == $t) {
        $found <- $mid
        $foundFlag <- 1
        $lo <- $hi+1
    }
    if($v[$mid] < $t) {
        $lo <- $mid+1
    }
    if($v[$mid] > $t) {
        $hi <- $mid-1
    }
}

if($foundFlag) {
    cp $found
} else {
    cp "not found"
}