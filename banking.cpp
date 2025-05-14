#include <iostream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string>
#include <cctype>
#include <iomanip>
// #include <ctime>
#include <limits>

using namespace std;

sql::mysql::MySQL_Driver *driver;
sql::Connection *con;

struct Account
{
    int acno;
    string name;
    int deposit;
    char type;
    string password;
};

void connect_to_database()
{
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "banking system");
    con->setSchema("class");
}

void create_account()
{
    Account acc;
    acc.acno = 1000;

    cout << "\n\n\t\t\tEnter Name: ";
    cin.ignore();
    getline(cin, acc.name);

    cout << "\t\t\tEnter Account Type (C/S): ";
    cin >> acc.type;
    acc.type = toupper(acc.type);

    cout << "\t\t\tInitial Deposit: ";
    cin >> acc.deposit;

    cout << "\t\t\tSet Password: ";
    cin >> acc.password;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("INSERT INTO accounts (name, deposit, type, password) VALUES (?, ?, ?, ?)");
        pstmt->setString(1, acc.name);
        pstmt->setInt(2, acc.deposit);
        pstmt->setString(3, string(1, acc.type));
        pstmt->setString(4, acc.password);
        pstmt->executeUpdate();

        delete pstmt;
        sql::Statement *stmt = con->createStatement();
        sql::ResultSet *res = stmt->executeQuery("SELECT LAST_INSERT_ID() AS acno");
        if (res->next())
        {
            acc.acno = res->getInt("acno");
            cout << "\n\t\t\tAccount created with account number: " << acc.acno << '\n';
        }

        delete res;
        delete stmt;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

bool authenticate(Account &acc)
{
    string input_pass;
    cout << "\t\t\tEnter password: ";
    cin >> input_pass;
    return acc.password == input_pass;
}

void show_account()
{
    int acno;
    cout << "\n\n\t\t\tEnter Account Number: ";
    cin >> acno;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
        pstmt->setInt(1, acno);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            Account acc;
            acc.acno = res->getInt("acno");
            acc.name = res->getString("name");
            acc.deposit = res->getInt("deposit");
            acc.type = res->getString("type")[0];
            acc.password = res->getString("password");

            if (authenticate(acc))
            {
                cout << "\n\t\t\tAccount No.: " << acc.acno;
                cout << "\n\t\t\tName: " << acc.name;
                cout << "\n\t\t\tType: " << acc.type;
                cout << "\n\t\t\tBalance: " << acc.deposit << endl;
            }
            else
            {
                cout << "\n\t\t\tInvalid password.\n";
            }
        }
        else
        {
            cout << "\n\t\t\tAccount not found.\n";
        }
        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

void deposit()
{
    int acno, amt;
    cout << "\n\t\t\tEnter Account Number: ";
    cin >> acno;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
        pstmt->setInt(1, acno);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            Account acc;
            acc.acno = res->getInt("acno");
            acc.name = res->getString("name");
            acc.deposit = res->getInt("deposit");
            acc.type = res->getString("type")[0];
            acc.password = res->getString("password");

            if (authenticate(acc))
            {
                cout << "\t\t\tAmount to Deposit: ";
                cin >> amt;
                acc.deposit += amt;
                sql::PreparedStatement *updateStmt = con->prepareStatement("UPDATE accounts SET deposit = ? WHERE acno = ?");
                updateStmt->setInt(1, acc.deposit);
                updateStmt->setInt(2, acc.acno);
                updateStmt->executeUpdate();
                cout << "\t\t\tDeposit successful. New Balance: " << acc.deposit << endl;

                sql::PreparedStatement *transStmt = con->prepareStatement("INSERT INTO transactions (from_account, to_account, amount, transaction_type) VALUES (?, ?, ?, ?)");
                transStmt->setInt(1, acc.acno);
                transStmt->setInt(2, acc.acno);
                transStmt->setInt(3, amt);
                transStmt->setString(4, "Deposit");
                transStmt->executeUpdate();

                delete updateStmt;
                delete transStmt;
            }
            else
            {
                cout << "\n\t\t\tInvalid password.\n";
            }
        }
        else
        {
            cout << "\n\t\t\tAccount not found.\n";
        }
        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

void withdraw()
{
    int acno, amt;
    cout << "\n\t\t\tEnter Account Number: ";
    cin >> acno;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
        pstmt->setInt(1, acno);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            Account acc;
            acc.acno = res->getInt("acno");
            acc.name = res->getString("name");
            acc.deposit = res->getInt("deposit");
            acc.type = res->getString("type")[0];
            acc.password = res->getString("password");

            if (authenticate(acc))
            {
                cout << "\t\t\tAmount to Withdraw: ";
                cin >> amt;
                if (acc.deposit >= amt)
                {
                    acc.deposit -= amt;
                    sql::PreparedStatement *updateStmt = con->prepareStatement("UPDATE accounts SET deposit = ? WHERE acno = ?");
                    updateStmt->setInt(1, acc.deposit);
                    updateStmt->setInt(2, acc.acno);
                    updateStmt->executeUpdate();
                    cout << "\t\t\tWithdrawal successful. New Balance: " << acc.deposit << endl;

                    sql::PreparedStatement *transStmt = con->prepareStatement("INSERT INTO transactions (from_account, to_account, amount, transaction_type) VALUES (?, ?, ?, ?)");
                    transStmt->setInt(1, acc.acno);
                    transStmt->setInt(2, acc.acno);
                    transStmt->setInt(3, amt);
                    transStmt->setString(4, "Withdraw");
                    transStmt->executeUpdate();

                    delete updateStmt;
                    delete transStmt;
                }
                else
                {
                    cout << "\t\t\tInsufficient balance.\n";
                }
            }
            else
            {
                cout << "\n\t\t\tInvalid password.\n";
            }
        }
        else
        {
            cout << "\n\t\t\tAccount not found.\n";
        }
        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

void modify_account()
{
    int acno;
    cout << "\n\t\t\tEnter Account Number: ";
    cin >> acno;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
        pstmt->setInt(1, acno);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            git
                Account acc;
            acc.acno = res->getInt("acno");
            acc.name = res->getString("name");
            acc.deposit = res->getInt("deposit");
            acc.type = res->getString("type")[0];
            acc.password = res->getString("password");

            if (authenticate(acc))
            {
                cout << "\t\t\tEnter New Name: ";
                cin.ignore();
                getline(cin, acc.name);
                cout << "\t\t\tNew Type (C/S): ";
                cin >> acc.type;

                cout << "\t\t\tNew Password: ";
                cin >> acc.password;

                sql::PreparedStatement *updateStmt = con->prepareStatement("UPDATE accounts SET name = ?, type = ?, password = ? WHERE acno = ?");
                updateStmt->setString(1, acc.name);
                updateStmt->setString(2, string(1, acc.type));

                updateStmt->setString(3, acc.password);
                updateStmt->setInt(4, acc.acno);
                updateStmt->executeUpdate();
                cout << "\t\t\tAccount updated.\n";

                delete updateStmt;
            }
            else
            {
                cout << "\n\t\t\tInvalid password.\n";
            }
        }
        else
        {
            cout << "\n\t\t\tAccount not found.\n";
        }
        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

void delete_account()
{
    int acno;
    cout << "\n\t\t\tEnter Account Number: ";
    cin >> acno;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
        pstmt->setInt(1, acno);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            Account acc;
            acc.acno = res->getInt("acno");
            acc.name = res->getString("name");
            acc.deposit = res->getInt("deposit");
            acc.type = res->getString("type")[0];
            acc.password = res->getString("password");

            if (authenticate(acc))
            {
                if (acc.deposit > 0)
                {
                    cout << "\n\t\t\tYou have birr of " << acc.deposit << " in your account.\n";
                    cout << "\t\t\tPlease withdraw all funds before deletion.\n";
                    delete pstmt;
                    delete res;
                    delete con;
                    return;
                }

                char confirm;
                cout << "\n\t\t\tAre you sure you want to delete your account? (Y/N): ";
                cin >> confirm;
                if (tolower(confirm) != 'y')
                {
                    cout << "\t\t\tAccount deletion cancelled.\n";
                    delete pstmt;
                    delete res;
                    delete con;
                    return;
                }

                sql::PreparedStatement *deleteTransStmt = con->prepareStatement(
                    "DELETE FROM transactions WHERE from_account = ? OR to_account = ?");
                deleteTransStmt->setInt(1, acno);
                deleteTransStmt->setInt(2, acno);
                deleteTransStmt->executeUpdate();
                cout << "\t\t\tTransactions deleted successfully.\n";
                delete deleteTransStmt;

                sql::PreparedStatement *deleteAccountStmt = con->prepareStatement(
                    "DELETE FROM accounts WHERE acno = ?");
                deleteAccountStmt->setInt(1, acno);
                deleteAccountStmt->executeUpdate();
                cout << "\t\t\tAccount deleted successfully.\n";
                delete deleteAccountStmt;
            }
            else
            {
                cout << "\n\t\t\tIncorrect password.\n";
            }
        }
        else
        {
            cout << "\n\t\t\tAccount not found.\n";
        }
        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

void show_all_accounts()
{
    string admin_pass;
    cout << "\n\t\t\tEnter Admin Password: ";
    cin >> admin_pass;

    if (admin_pass != "fkr123")
    {
        cout << "\t\t\tAccess denied.\n";
        return;
    }

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts");
        sql::ResultSet *res = pstmt->executeQuery();

        cout << "\n\t\t\tAll Accounts:\n";
        cout << "===============================================\n";
        cout << "AccNo\tName\t\tType\tBalance\n";
        while (res->next())
        {
            cout << res->getInt("acno") << "\t" << res->getString("name") << "\t\t"
                 << res->getString("type") << "\t" << res->getInt("deposit") << endl;
        }

        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

void transfer()
{
    int from, to, amt;
    cout << "\n\t\t\tEnter Sender Account Number: ";
    cin >> from;

    try
    {
        connect_to_database();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
        pstmt->setInt(1, from);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            Account sender;
            sender.acno = res->getInt("acno");
            sender.name = res->getString("name");
            sender.deposit = res->getInt("deposit");
            sender.type = res->getString("type")[0];
            sender.password = res->getString("password");

            if (authenticate(sender))
            {
                cout << "\t\t\tEnter Receiver Account Number: ";
                cin >> to;
                cout << "\t\t\tAmount to Transfer: ";
                cin >> amt;

                if (sender.deposit >= amt)
                {
                    sql::PreparedStatement *pstmt2;
                    pstmt2 = con->prepareStatement("SELECT * FROM accounts WHERE acno = ?");
                    pstmt2->setInt(1, to);
                    sql::ResultSet *res2 = pstmt2->executeQuery();

                    if (res2->next())
                    {
                        Account receiver;
                        receiver.acno = res2->getInt("acno");
                        receiver.name = res2->getString("name");
                        receiver.deposit = res2->getInt("deposit");
                        receiver.type = res2->getString("type")[0];

                        sender.deposit -= amt;
                        sql::PreparedStatement *updateSenderStmt = con->prepareStatement("UPDATE accounts SET deposit = ? WHERE acno = ?");
                        updateSenderStmt->setInt(1, sender.deposit);
                        updateSenderStmt->setInt(2, sender.acno);
                        updateSenderStmt->executeUpdate();

                        receiver.deposit += amt;
                        sql::PreparedStatement *updateReceiverStmt = con->prepareStatement("UPDATE accounts SET deposit = ? WHERE acno = ?");
                        updateReceiverStmt->setInt(1, receiver.deposit);
                        updateReceiverStmt->setInt(2, receiver.acno);
                        updateReceiverStmt->executeUpdate();

                        cout << "\t\t\tTransfer successful.\n";

                        sql::PreparedStatement *transStmt = con->prepareStatement("INSERT INTO transactions (from_account, to_account, amount, transaction_type) VALUES (?, ?, ?, ?)");
                        transStmt->setInt(1, sender.acno);
                        transStmt->setInt(2, receiver.acno);
                        transStmt->setInt(3, amt);
                        transStmt->setString(4, "Transfer");
                        transStmt->executeUpdate();

                        delete updateSenderStmt;
                        delete updateReceiverStmt;
                        delete transStmt;
                    }
                    else
                    {
                        cout << "\t\t\tReceiver account not found.\n";
                    }

                    delete pstmt2;
                    delete res2;
                }
                else
                {
                    cout << "\t\t\tInsufficient balance.\n";
                }
            }
            else
            {
                cout << "\n\t\t\tInvalid password.\n";
            }
        }
        else
        {
            cout << "\n\t\t\tSender account not found.\n";
        }
        delete pstmt;
        delete res;
        delete con;
    }
    catch (sql::SQLException &e)
    {
        cout << "\t\t\tError: " << e.what() << endl;
    }
}

int main()
{
    int choice;
    do
    {
        cout << "\n\t\t\t*** Banking System ***\n";
        cout << "\n\t\t\t1. Create Account\n";
        cout << "\t\t\t2. Show Account\n";
        cout << "\t\t\t3. Deposit\n";
        cout << "\t\t\t4. Withdraw\n";
        cout << "\t\t\t5. Modify Account\n";
        cout << "\t\t\t6. Delete Account\n";
        cout << "\t\t\t7. Show All Accounts\n";
        cout << "\t\t\t8. Transfer\n";
        cout << "\t\t\t9. Exit\n";
        cout << "\t\t\tEnter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            create_account();
            break;
        case 2:
            show_account();
            break;
        case 3:
            deposit();
            break;
        case 4:
            withdraw();
            break;
        case 5:
            modify_account();
            break;
        case 6:
            delete_account();
            break;
        case 7:
            show_all_accounts();
            break;
        case 8:
            transfer();
            break;
        case 9:
            cout << "\n\t\t\tThank you for using the Banking System!\n";
            break;
        default:
            cout << "\n\t\t\tInvalid choice.\n";
        }
    } while (choice != 9);
}