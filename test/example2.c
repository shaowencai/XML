#include<stdio.h>
#include "../src/xml.h"

int main()
{
	int fd;
	char xmlString[4096*10];
	char mallocBuf[4096*5];
	xml_node_t *root;
	xml_node_t *node;
	int i,j = 0;
	int num = 0;
	
	fd = open("PMC-53MV10.AnyPollingMaster.xml", O_RDONLY); 
	
	read(fd, xmlString, 12847);
	
	close(fd);
	
	root = xml_parse_document(xmlString, strlen(xmlString),mallocBuf,sizeof(mallocBuf));
	
	printf("---- %s\r\n",xml_node_name(root));
	
	num = xml_node_child_num(root);
	
	for(i=0; i<num; i++)
	{
		node = xml_get_node_nth_child(root,i);
		printf("---- name:%s\r\n",xml_node_name(node));
		printf("---- content:%s\r\n",xml_node_content(node));
		printf("---- attr_num:%d\r\n",xml_node_attr_num(node));
		for(j=0; j<xml_node_attr_num(node); j++)
		{
			printf("---- %s:%s\r\n",xml_get_node_nth_attr_key(node,j),xml_get_node_nth_attr_value(node,j));
		}
	}
	node = xml_get_node_nth_child(root,1);
	root = xml_get_first_node_by_depth(node,1);
	root = xml_get_node_nth_child(root,1);
	
	num = xml_node_child_num(root);
		
	for(i=0; i<num; i++)
	{
		node = xml_get_node_nth_child(root,i);
		printf("---- name:%s\r\n",xml_node_name(node));
		printf("---- content:%s\r\n",xml_node_content(node));
		printf("---- attr_num:%d\r\n",xml_node_attr_num(node));
		for(j=0; j<xml_node_attr_num(node); j++)
		{
			printf("---- %s:%s\r\n",xml_get_node_nth_attr_key(node,j),xml_get_node_nth_attr_value(node,j));
		}
	}
	return 1;
}
