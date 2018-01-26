#include <bits/stdc++.h>

using namespace std;
#define inf 1000000000
#define unvisited -1
#define visited 1
#define eps 1e-9
#define mp make_pair
#define pb push_back
#define pi acos(-1.0)
#define uint64 unsigned long long
typedef long long ll;
typedef vector<int> vi;
typedef pair<int,int> ii;
typedef vector<ii> vii;

int main(){
	string input[400];
	int hasil[400][400];
	int i,j;
	i=0;
	memset(hasil,0,sizeof hasil);
	
	freopen("../jet.txt","r",stdin);
	while(getline(cin,input[i])){
		for(j=0;j<input[i].length();j++){
			if(input[i][j]!=' '){hasil[i][j] = 0;}
			else{hasil[i][j] = 1;}
		}
		i++;
	}
	freopen("../outputJet.txt","w",stdout);
	int idx = i;
	for(i=0;i<idx;i++){
		for(j=0;j<input[i].length();j++){
			printf("%d",hasil[i][j]);
		}
		printf("\n");
	}
	return 0;
};