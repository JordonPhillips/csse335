//Write a program that asks the user to type an integer N and that indicates if N is a prime number or not.

#include<stdio.h>
#include<stdbool.h>

bool isprime(int n){
  if (n == 1) return false;
  int i, j;
  for (i=2; i<n ;i++){
    if(n%i==0)
	  if (n%i==0 && i != n) return false;
  }
  return true;
}

int main(){
    int n;
    int prime=1;
    printf("Write a number: ");
    scanf("%d",&n);
    if (isprime(n)) {
        printf("%d is prime\n",n);
    } else {
        printf("%d is not prime.\n",n);
    }
    return 0;
}
