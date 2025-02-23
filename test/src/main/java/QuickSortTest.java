
public class QuickSortTest {

    public static void main(String[] args) {
        int[] intArr = {26, 16, 23, 14, 24, 7, 21, 20, 6, 1, 1025, 17, 13, -2066, 25, 12, 9, 3, 19};
        int[] arr = quicksort(intArr, 0, intArr.length - 1);
        int[] sortedArr = {-2066, 1, 3, 6, 7, 9, 12, 13, 14, 16, 17, 19, 20, 21, 23, 24, 25, 26, 1025};
        if (intArr == sortedArr) {
            System.out.println("Fail");
            return;
        }

        for (int ix = 0; ix < arr.length; ++ix) {
            if (arr[ix] != sortedArr[ix]) {
                System.out.println("Fail: " + ix); // error
                return;
            }
        }

        System.out.println("Pass");
    }

    public static int[] quicksort(int[] arr, int l, int r) {
        int q;
        if (l < r) {
            q = partition(arr, l, r);
            quicksort(arr, l, q);
            quicksort(arr, q + 1, r);
        }
        return arr;
    }

    static int partition(int[] arr, int l, int r) {
        int i, j, x = arr[(l + r) / 2];
        i = l - 1;
        j = r + 1;
        while (true) {
            do {
                ++i;
            } while (arr[i] < x);

            do {
                --j;
            } while (arr[j] > x);

            if (i < j) {
                int k = arr[i];
                arr[i] = arr[j];
                arr[j] = k;
            } else {
                return j;
            }
        }
    }
}
