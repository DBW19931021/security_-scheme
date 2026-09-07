#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Flash配置生成器
从Excel表格读取烧录配置，生成JSON配置文件
支持动态添加customer_id和配置项
"""

import pandas as pd
import json
import argparse
import sys
from pathlib import Path
from datetime import datetime
import re

class FlashConfigGenerator:
    """Flash配置生成器"""
    
    def __init__(self):
        self.supported_flash_types = ['bootloader', 'firmware', 'host', 'server']
    
    def create_excel_template(self, output_file):
        """创建Excel模板文件"""
        # 示例数据
        data = [
            {'customer_id': 'b000', 'flash_type': 'bootloader', 'command': 'python3 ehsm_fw_loader.py', 
             'args': '${WORKSPACE}/${DIR_APP}/build/ehsm_bl_with_sig.bin', 'description': 'Load bootloader for b000 customer'},
            {'customer_id': 'b000', 'flash_type': 'firmware', 'command': 'python3 ehsm_fw_loader.py', 
             'args': '${WORKSPACE}/${DIR_APP}/build/ehsm_fw.bin', 'description': 'Load firmware for b000 customer'},
            {'customer_id': 'b000', 'flash_type': 'host', 'command': 'python3 flash_loader.py', 
             'args': '-t cm3 -m swd', 'description': 'Flash host for b000 customer using CM3 SWD'},
            {'customer_id': 'b000', 'flash_type': 'server', 'command': './build.sh', 
             'args': 'port/cm3 -c', 'description': 'Build server for b000 customer using CM3 platform'},
            {'customer_id': '0000', 'flash_type': 'bootloader', 'command': 'python3 ehsm_fw_loader.py', 
             'args': '${WORKSPACE}/${DIR_APP}/build/ehsm_bl_with_sig.bin', 'description': 'Load bootloader for 0000 customer'},
            {'customer_id': '0000', 'flash_type': 'firmware', 'command': 'python3 ehsm_fw_loader.py', 
             'args': '${WORKSPACE}/${DIR_APP}/build/ehsm_fw.bin --fastbl ${WORKSPACE}/${DIR_TEST}/server/scripts/fast_bl.bin', 'description': 'Load firmware for 0000 customer with FastBL'},
            {'customer_id': '0000', 'flash_type': 'host', 'command': 'python3 flash_loader.py', 
             'args': '-t m130 -m jtag', 'description': 'Flash host for 0000 customer using M130 JTAG'},
            {'customer_id': '0000', 'flash_type': 'server', 'command': './build.sh', 
             'args': 'port/m130 -c', 'description': 'Build server for 0000 customer using M130 platform'},
            {'customer_id': '4030', 'flash_type': 'bootloader', 'command': 'python3 ehsm_fw_loader.py', 
             'args': '${WORKSPACE}/${DIR_APP}/build/ehsm_bl_with_sig.bin', 'description': 'Load bootloader for 4030 customer'},
            {'customer_id': '4030', 'flash_type': 'firmware', 'command': 'python3 ehsm_fw_loader.py', 
             'args': '${WORKSPACE}/${DIR_APP}/build/ehsm_fw.bin --fastbl ${WORKSPACE}/${DIR_TEST}/server/scripts/fast_bl.bin', 'description': 'Load firmware for 4030 customer with FastBL'},
            {'customer_id': '4030', 'flash_type': 'host', 'command': 'python3 flash_loader.py', 
             'args': '-t m130 -m jtag', 'description': 'Flash host for 4030 customer using M130 JTAG'},
            {'customer_id': '4030', 'flash_type': 'server', 'command': './build.sh', 
             'args': 'port/m130 -c', 'description': 'Build server for 4030 customer using M130 platform'},
        ]
        
        df = pd.DataFrame(data)
        
        # 创建Excel文件并添加格式
        with pd.ExcelWriter(output_file, engine='openpyxl') as writer:
            df.to_excel(writer, sheet_name='Flash_Config', index=False)
            
            # 获取工作表和工作簿
            workbook = writer.book
            worksheet = writer.sheets['Flash_Config']
            
            # 设置列宽
            column_widths = {'A': 15, 'B': 15, 'C': 25, 'D': 60, 'E': 40}
            for column, width in column_widths.items():
                worksheet.column_dimensions[column].width = width
                
            # 添加数据验证（flash_type列）
            from openpyxl.worksheet.datavalidation import DataValidation
            flash_type_validation = DataValidation(
                type="list", 
                formula1='"bootloader,firmware,host,server"',
                showErrorMessage=True,
                errorTitle="无效的flash_type",
                error="请选择: bootloader, firmware, host, server"
            )
            worksheet.add_data_validation(flash_type_validation)
            flash_type_validation.add('B2:B100')  # 应用到B列
            
        print(f"[OK] Excel模板已创建: {output_file}")
        return True
    
    def parse_args_string(self, args_string):
        """解析参数字符串为参数列表"""
        if not args_string.strip():
            return []
        
        # 使用正则表达式分割，保持引号内的内容完整
        args = re.findall(r'[^\s"\']+|"[^"]*"|\'[^\']*\'', args_string)
        # 去除引号
        cleaned_args = []
        for arg in args:
            if (arg.startswith('"') and arg.endswith('"')) or (arg.startswith("'") and arg.endswith("'")):
                cleaned_args.append(arg[1:-1])
            else:
                cleaned_args.append(arg)
        return cleaned_args
    
    def validate_data(self, df):
        """验证数据有效性"""
        errors = []
        
        # 检查必需列
        required_columns = ['customer_id', 'flash_type', 'command', 'args', 'description']
        missing_columns = [col for col in required_columns if col not in df.columns]
        if missing_columns:
            errors.append(f"缺少必需列: {missing_columns}")
        
        # 检查flash_type有效性
        invalid_flash_types = df[~df['flash_type'].isin(self.supported_flash_types)]['flash_type'].unique()
        if len(invalid_flash_types) > 0:
            errors.append(f"无效的flash_type: {invalid_flash_types}，支持的类型: {self.supported_flash_types}")
        
        # 检查空值
        for col in required_columns:
            if col in df.columns:
                null_count = df[col].isnull().sum()
                if null_count > 0:
                    errors.append(f"列 '{col}' 中有 {null_count} 个空值")
        
        return errors
    
    def generate_json_config(self, excel_file, output_file=None):
        """从Excel文件生成JSON配置"""
        try:
            # 读取Excel文件
            if excel_file.endswith('.xlsx') or excel_file.endswith('.xls'):
                df = pd.read_excel(excel_file, sheet_name='Flash_Config')
            else:
                df = pd.read_csv(excel_file)
            
            print(f"[READ] 读取配置文件: {excel_file}")
            print(f"[INFO] 找到 {len(df)} 条配置记录")
            
            # 数据验证
            errors = self.validate_data(df)
            if errors:
                print("[ERROR] 数据验证失败:")
                for error in errors:
                    print(f"   - {error}")
                return False
            
            # 构建配置结构
            flash_configurations = {}
            customer_ids = set()
            
            for _, row in df.iterrows():
                customer_id = str(row['customer_id']).strip()
                flash_type = str(row['flash_type']).strip()
                command = str(row['command']).strip()
                args_string = str(row['args']).strip()
                description = str(row['description']).strip()
                
                customer_ids.add(customer_id)
                
                if customer_id not in flash_configurations:
                    flash_configurations[customer_id] = {}
                
                # 解析参数
                args = self.parse_args_string(args_string)
                
                flash_configurations[customer_id][flash_type] = {
                    "command": command,
                    "args": args,
                    "description": description
                }
            
            # 构建完整配置
            config = {
                "flash_configurations": flash_configurations,
                "metadata": {
                    "version": "1.0.1",
                    "description": "Flash and build configuration for different customer IDs",
                    "last_updated": datetime.now().strftime("%Y-%m-%d"),
                    "supported_customers": sorted(list(customer_ids)),
                    "flash_types": self.supported_flash_types,
                    "generated_from": Path(excel_file).name
                }
            }
            
            # 输出JSON文件
            if output_file is None:
                output_file = Path(excel_file).parent / "flash_config.json"
            
            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(config, f, indent=2, ensure_ascii=False)
            
            print(f"[OK] JSON配置文件已生成: {output_file}")
            print(f"[INFO] 支持的客户: {sorted(list(customer_ids))}")
            print(f"[INFO] 配置类型: {self.supported_flash_types}")
            
            return True
            
        except Exception as e:
            print(f"[ERROR] 生成JSON配置失败: {e}")
            return False

def main():
    parser = argparse.ArgumentParser(description='Flash配置生成器')
    parser.add_argument('action', choices=['template', 'generate'], 
                       help='操作类型: template(创建Excel模板) 或 generate(生成JSON配置)')
    parser.add_argument('-i', '--input', help='输入Excel文件路径')
    parser.add_argument('-o', '--output', help='输出文件路径')
    
    args = parser.parse_args()
    
    generator = FlashConfigGenerator()
    
    if args.action == 'template':
        output_file = args.output or 'flash_config_template.xlsx'
        success = generator.create_excel_template(output_file)
        if success:
            print(f"\n[SUCCESS] Excel模板创建完成！")
            print(f"[INFO] 请编辑 {output_file} 文件来配置您的烧录设置")
            print(f"[HINT] 编辑完成后，运行: python {sys.argv[0]} generate -i {output_file}")
    
    elif args.action == 'generate':
        if not args.input:
            print("[ERROR] 生成JSON配置需要指定输入文件 (-i 参数)")
            return False
        
        if not Path(args.input).exists():
            print(f"[ERROR] 输入文件不存在: {args.input}")
            return False
        
        success = generator.generate_json_config(args.input, args.output)
        if success:
            print(f"\n[SUCCESS] JSON配置生成完成！")
            print(f"[INFO] 现在可以使用新的烧录配置运行Jenkins Pipeline")

if __name__ == "__main__":
    main()