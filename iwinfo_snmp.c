#include "iwinfo.h"
#include <stdio.h>
#include <string.h>

/* iwinfo snmp interface
 * 
 * .1.3.9950.1.1:   2.4GHz Associations
 * .1.3.9950.1.2:   5  GHz Associations
 * .1.3.9950.2.1:   2.4GHz Noise
 * .1.3.9950.2.2:   5  GHz Noise
 */

const char *base_oid = ".1.3.9950";
const int supported_count = 4;
const char *supported_oids[] =
{
	".1.1",
	".1.2",
	".2.1",
	".2.2"
};

void get_assoc(char *interface)
{
	const struct iwinfo_ops *iw = iwinfo_backend(interface);
	int len;
	char buf[IWINFO_BUFSIZE];
	int rv = 0;

	if (!iw)
	{
		fprintf(stderr, "No such wireless device: %s\n", interface);
		rv = 1;
	}
	if (rv == 0 && iw->assoclist(interface, buf, &len))
	{
		fprintf(stderr, "No information available\n");
		rv = 1;
	}
	
	if(rv == 0) printf("%ld\n", len / sizeof(struct iwinfo_assoclist_entry));
	
	iwinfo_finish();
}

void get_noise(char *interface)
{
	const struct iwinfo_ops *iw = iwinfo_backend(interface);
	int noise, rv = 0;
	
	if (!iw)
	{
		fprintf(stderr, "No such wireless device: %s\n", interface);
		rv = 1;
	}
	
	if(rv == 0)
	{
		if (iw->noise(interface, &noise)) noise = 0;
		printf("%d\n", noise);
	}
	
	iwinfo_finish();
}

bool get(char *oid)
{
	if(oid == NULL || strstr(oid, base_oid) != oid) return false;
	
	bool ret = true;
	int base_oid_len = strlen(base_oid);
	int suboid_len = strlen(oid) - base_oid_len + 1;
	char *suboid = malloc(suboid_len);
	if (suboid == NULL) return false;
	memcpy(suboid, oid + base_oid_len, suboid_len);
	
	if(strcmp(suboid, supported_oids[0]) == 0)
	{
		printf("%s\ninteger\n", oid);
		get_assoc("2.4GHz");
	}
	else if(strcmp(suboid, supported_oids[1]) == 0)
	{
		printf("%s\ninteger\n", oid);
		get_assoc("5GHz");
	}
	else if(strcmp(suboid, supported_oids[2]) == 0)
	{
		printf("%s\ninteger\n", oid);
		get_noise("2.4GHz");
	}
	else if(strcmp(suboid, supported_oids[3]) == 0)
	{
		printf("%s\ninteger\n", oid);
		get_noise("5GHz");
	}
	else
	{
		ret = false;
	}
	
	free(suboid);
	return ret;
}

bool get_next(char *oid)
{
	if(oid == NULL || strstr(oid, base_oid) != oid) return false;
	
	char retstr[20] = { 0 };
	int base_oid_len = strlen(base_oid);
	int suboid_len = strlen(oid) - base_oid_len + 1;
	int matching_oid = 0;
	bool found = false;

	char* suboid = malloc(suboid_len);
	if (suboid == NULL) return false;

	memcpy(suboid, oid + base_oid_len, suboid_len);
	memcpy(retstr, base_oid, base_oid_len);

	for (int i = 0; !found && i < supported_count; i++)
	{
		int loop_len = (strlen(supported_oids[i]) < suboid_len - 1 ? strlen(supported_oids[i]) : suboid_len - 1);

		for (int j = 0; j < loop_len; j++)
		{
			if (suboid[j] == '.') continue;
			if (suboid[j] < supported_oids[i][j])
			{
				found = true;
				break;
			}
			if (suboid[j] > supported_oids[i][j] || (suboid[j] == supported_oids[i][j] && j == strlen(supported_oids[i]) - 1))
			{
				matching_oid++;
				break;
			}
		}
	}
	
	free(suboid);
	if(matching_oid < supported_count)
	{
		strcat(retstr, supported_oids[matching_oid]);
		get(retstr);
		return true;
	}
	else
	{
		return false;
	}
}

int main()
{
	setbuf(stdout, NULL);
	
	char *line = NULL;
	size_t len = 0;
    ssize_t read = 0;
	
	while(read != -1)
	{
		read = getline(&line, &len, stdin);
		if(read == -1) continue;
		
		if(strcmp(line, "PING\n") == 0)
		{
			printf("PONG\n");
		}
		else if(strcmp(line, "set\n") == 0)
		{
			printf("not-writable\n");
		}
		else if(strcmp(line, "get\n") == 0 || strcmp(line, "getnext\n") == 0)
		{
			bool is_get = (strcmp(line, "get\n") == 0);

			len = 0;
			free(line);
			line = NULL;
			read = getline(&line, &len, stdin);
			if(read == -1) continue;
			for(int i = 0; i < len; i++) if(line[i] < 32) line[i] = 0;
			
			if(!(is_get ? get(line) : get_next(line)))
			{
				printf("NONE\n");
			}
		}
		
		// Silently ignore unhandled commands
		// Clean up for next round
		len = 0;
		free(line);
		line = NULL;
	}
	
	free(line); // Just in case
	return 0;
}
