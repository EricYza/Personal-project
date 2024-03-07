import sys

def write_user_info(username, password):
    with open('user.txt', 'a') as file:
        file.write(f'{username} ')
        file.write(f'{password}\n')

def main():
    if len(sys.argv) != 3:
        print("Usage: python sudo.py <username> <password>")
        sys.exit(1)

    username = sys.argv[1]
    password = sys.argv[2]
    write_user_info(username, password)
    print("User information saved to user.txt")

if __name__ == "__main__":
    main()
