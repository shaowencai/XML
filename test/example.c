#include<stdio.h>
#include "../src/xml.h"

int main()
{
	char xmlString[2018] = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Root>\r\n<!-- header节点各属性值 -->\r\n<!-- version固定。由驱值为1.0 -->\r\n<!-- author:编辑xml的作者；file约.xml” -->\r\n<!-- filename:对于AnyPolling，规约固定为0 -->\r\n<Hello id=\"1\" name=\"000123456789-1090\" coding=\"XXX\" error=\"XXX\" sample_time=\"YYYYMMDDHHMMSS\">World</Hello>\r\n<!-- header节点各属性值 -->\r\n<This>\r\n<!-- header节点各属性值 -->\r\n<Is>:-)</Is><An>shaowencailalalalalalO</An><Example>:-D</Example>\r\n</This>\r\n</Root>";
	char mallocBuf[4096];
	xml_node_t *root;
	xml_node_t *node;
	int i,j = 0;
	int num = 0;
	
	printf("%s\r\n\r\n",xmlString);
	
	root = xml_parse_document(xmlString, strlen(xmlString),mallocBuf,sizeof(mallocBuf));
	
	printf("---- %s\r\n",xml_node_name(root));
	
	num = xml_node_child_num(root);
	
	for(i=0; i<num; i++)
	{
		node = xml_get_node_nth_child(root,i);
		printf("---- name:    %s\r\n",xml_node_name(node));
		printf("---- content: %s\r\n",xml_node_content(node));
		printf("---- attr_num:%d\r\n",xml_node_attr_num(node));
		for(j=0; j<xml_node_attr_num(node); j++)
		{
			printf("---- attr_name: %s\r\n",xml_get_node_nth_attr_key(node,j));
			printf("---- attr_value:%s\r\n",xml_get_node_nth_attr_value(node,j));
		}
	}
	
	return 1;
}
