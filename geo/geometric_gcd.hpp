// 几何辗转相除法函数
// a, b: 两个实数长度
// eps: 允许的精度误差，比如 1e-9
template <typename T>
pair<bool, T> geometricGCD(T a, T b, T eps = 1e-9) {

    int step = 1;
    // 确保 a 是较长的那根
    if (a < b) std::swap(a, b);

    while (b > eps) {
        // 计算当前较短线段可以截取多少个整段
        int count = static_cast<int>(a / b);
        // 计算剩下的余数线段 R
        double r = a - count * b;


        // 迭代：长线段变成原来的短线段，短线段变成余数
        a = b;
        b = r;

        // 防御性跳出：如果循环次数过多，说明大概率是不可通约的无理数
        if (step > 15) {
            return mp(0, a);
        }
    }
    return mp(1, a);

}