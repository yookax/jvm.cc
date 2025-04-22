/**
 * JEP 358: NullPointerException
 * 该特性改进了 NullPointerException 的可读性，能更准确地给出 null变量的信息。
 */
public class NullPointerExceptionClearMsg {
    public static void main(String[] args) {
//        Bank bank = new Bank(new Customer(new Account(1000)));
        /*
        Exception in thread "main" java.lang.NullPointerException: Cannot invoke
        "com.atguigu.feature.Account.withdraw(double)" because the return value of
        "com.atguigu.feature.Customer.getAccount()" is null
         */
        Bank bank = new Bank(new Customer());

        bank.getCustomer().getAccount().withdraw(200);
    }

    private static class Bank {
        private Customer customer;

        public Bank() {
        }

        public Bank(Customer customer) {
            this.customer = customer;
        }

        public Customer getCustomer() {
            return customer;
        }

        public void setCustomer(Customer customer) {
            this.customer = customer;
        }
    }

    private static class Customer {
        private Account account;

        public Customer() {
        }

        public Customer(Account account) {
            this.account = account;
        }

        public Account getAccount() {
            return account;
        }

        public void setAccount(Account account) {
            this.account = account;
        }
    }

    private static class Account {
        private double balance;//余额

        public Account() {
        }

        public Account(double balance) {
            this.balance = balance;
        }

        //取钱操作
        public void withdraw(double amt) {
            if (balance >= amt) {
                balance -= amt;
                System.out.println("成功取款：" + amt);
            } else {
                System.out.println("余额不足，取款失败");
            }
        }
    }
}
