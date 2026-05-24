$v[] <- input.in
$doSort <- do-sort.in
$i <- 0

# Sorting
if($doSort) {
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