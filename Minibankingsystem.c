#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "accounts.dat"
#define LOG_FILE "transactions.log"

struct Account {
    int accNo;
    char name[50];
    char password[20];
    float balance;
};

// ================= FILE HELPERS =================

void logTransaction(const char *msg) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp == NULL) return;
    fprintf(fp, "%s\n", msg);
    fclose(fp);
}

// ================= ACCOUNT FUNCTIONS =================

int accountExists(int accNo) {
    struct Account acc;
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) return 0;

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accNo == accNo) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void createAccount() {
    struct Account acc;

    printf("\n--- CREATE ACCOUNT ---\n");
    printf("Enter Account Number: ");
    scanf("%d", &acc.accNo);

    if (accountExists(acc.accNo)) {
        printf("Account already exists!\n");
        return;
    }

    printf("Enter Name: ");
    scanf(" %[^\n]", acc.name);

    printf("Set Password: ");
    scanf("%s", acc.password);

    acc.balance = 0;

    FILE *fp = fopen(FILE_NAME, "ab");
    fwrite(&acc, sizeof(acc), 1, fp);
    fclose(fp);

    printf("Account created successfully!\n");
}

// ================= LOGIN =================

int login(struct Account *loggedIn) {
    int accNo;
    char password[20];
    struct Account acc;

    printf("\n--- LOGIN ---\n");
    printf("Account Number: ");
    scanf("%d", &accNo);

    printf("Password: ");
    scanf("%s", password);

    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) return 0;

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accNo == accNo && strcmp(acc.password, password) == 0) {
            *loggedIn = acc;
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// ================= UPDATE ACCOUNT =================

void updateAccount(struct Account acc) {
    FILE *fp = fopen(FILE_NAME, "rb");
    FILE *temp = fopen("temp.dat", "wb");

    struct Account a;

    while (fread(&a, sizeof(a), 1, fp)) {
        if (a.accNo == acc.accNo)
            fwrite(&acc, sizeof(acc), 1, temp);
        else
            fwrite(&a, sizeof(a), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.dat", FILE_NAME);
}

// ================= OPERATIONS =================

void deposit(struct Account *acc) {
    float amount;
    printf("Enter amount to deposit: ");
    scanf("%f", &amount);

    acc->balance += amount;

    char msg[100];
    sprintf(msg, "Deposit: +%.2f (Account %d)", amount, acc->accNo);
    logTransaction(msg);

    updateAccount(*acc);
    printf("Deposit successful!\n");
}

void withdraw(struct Account *acc) {
    float amount;
    printf("Enter amount to withdraw: ");
    scanf("%f", &amount);

    if (amount > acc->balance) {
        printf("Insufficient balance!\n");
        return;
    }

    acc->balance -= amount;

    char msg[100];
    sprintf(msg, "Withdraw: -%.2f (Account %d)", amount, acc->accNo);
    logTransaction(msg);

    updateAccount(*acc);
    printf("Withdrawal successful!\n");
}

void transfer(struct Account *acc) {
    int targetAcc;
    float amount;

    printf("Enter target account: ");
    scanf("%d", &targetAcc);

    if (!accountExists(targetAcc)) {
        printf("Target account not found!\n");
        return;
    }

    printf("Enter amount: ");
    scanf("%f", &amount);

    if (amount > acc->balance) {
        printf("Insufficient balance!\n");
        return;
    }

    // deduct from sender
    acc->balance -= amount;
    updateAccount(*acc);

    // add to receiver
    FILE *fp = fopen(FILE_NAME, "rb+");
    struct Account temp;

    while (fread(&temp, sizeof(temp), 1, fp)) {
        if (temp.accNo == targetAcc) {
            temp.balance += amount;
            fseek(fp, -sizeof(temp), SEEK_CUR);
            fwrite(&temp, sizeof(temp), 1, fp);
            break;
        }
    }
    fclose(fp);

    char msg[150];
    sprintf(msg, "Transfer: %.2f from %d to %d", amount, acc->accNo, targetAcc);
    logTransaction(msg);

    printf("Transfer successful!\n");
}

void viewAccount(struct Account acc) {
    printf("\n--- ACCOUNT DETAILS ---\n");
    printf("Account No: %d\n", acc.accNo);
    printf("Name: %s\n", acc.name);
    printf("Balance: %.2f\n", acc.balance);
}

// ================= DASHBOARD =================

void dashboard(struct Account acc) {
    int choice;

    do {
        printf("\n===== BANK DASHBOARD =====\n");
        printf("1. View Account\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Transfer\n");
        printf("5. Logout\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: viewAccount(acc); break;
            case 2: deposit(&acc); break;
            case 3: withdraw(&acc); break;
            case 4: transfer(&acc); break;
        }

    } while (choice != 5);
}

// ================= MAIN =================

int main() {
    int choice;
    struct Account acc;

    while (1) {
        printf("\n===== MINI BANK SYSTEM =====\n");
        printf("1. Create Account\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createAccount();
                break;

            case 2:
                if (login(&acc)) {
                    printf("Login successful!\n");
                    dashboard(acc);
                } else {
                    printf("Invalid credentials!\n");
                }
                break;

            case 3:
                exit(0);
        }
    }

    return 0;
}