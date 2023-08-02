`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/07/31 13:29:12
// Design Name: 
// Module Name: fifo_ctrl
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module fifo_ctrl(
    input clk,
    input rst_n,

    input rec_fifo_wren,
    input [31:0] rec_fifo_din,
    input rec_pkt_done,
    input [15:0] rec_byte_num,

    input tx_fifo_rden,
    output [31:0] tx_fifo_dout,
    input tx_pkt_done,
    output tx_start_en,
    output [15:0] tx_byte_num
    );


//定时累加器，硬件上电后即一直开始工作
reg [63:0] timerCnt;
always @(posedge clk, negedge rst_n)begin
	if(!rst_n)begin 
		timerCnt <= 64'd0;
	end 
	else begin
		timerCnt <= timerCnt + 64'd1;
	end	
end

// 实际使用时需要分开UDP的RX/TX过程, 是相互独立的 
// 这里为了测试loop, 简单的将done和en信号直连, 形态都为1个clk的有效脉冲
// PC软件中会强制先下发数据然后等待上行数据, 再做比较.然后再下发数据, 依次循环执行
assign tx_start_en = rec_pkt_done;
assign tx_byte_num = rec_byte_num;

reg [63:0] tPcToFpgaEnd;		//FPGA读取PC下发数据结束
reg [63:0] tFpgaToPcEnd;		//FPGA写入CP上传数据结束
reg catch;							//计算各个时间段的节点
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        tPcToFpgaEnd <= 0;
        tFpgaToPcEnd <= 0;
        catch <= 1'b0;
    end
    else if(rec_pkt_done) begin
        tPcToFpgaEnd <= timerCnt;
    end
    else if(tx_pkt_done) begin
        tFpgaToPcEnd <= timerCnt;
        catch <= 1'b1;
    end
    else begin
        catch <= 1'b0;
    end
end

reg [63:0] tCurPcToFpgaEnd;
reg [63:0] tCurFpgaToPcEnd;
reg [63:0] tLastFpgaToPcEnd;
(* MARK_DEBUG="true" *) reg [63:0] tPcLoop;
(* MARK_DEBUG="true" *) reg [63:0] tFpgaWrite;
(* MARK_DEBUG="true" *) reg [63:0] tMaxPcLoop;
reg [2:0] tstate;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
		tCurPcToFpgaEnd <= 64'd0;
		tCurFpgaToPcEnd <= 64'd0;
        tLastFpgaToPcEnd <= 64'd0;
        tPcLoop <= 64'd0;
        tFpgaWrite <= 64'd0;
        tMaxPcLoop <= 64'd0;
        tstate <= 3'd0;
    end
    else begin
		case(tstate)
		0: begin
			//在catch时刻暂存所有时间数据
			if (catch==1'b1) begin
				tCurPcToFpgaEnd <= tPcToFpgaEnd;
				tCurFpgaToPcEnd <= tFpgaToPcEnd;
				tstate <= tstate + 1'b1;
			end
		end

			//计算时间长度
		1: begin
			tFpgaWrite <= tCurFpgaToPcEnd - tCurPcToFpgaEnd;
			tPcLoop <= tCurPcToFpgaEnd - tLastFpgaToPcEnd;       //将FPGA解析UDP包的时间包含在tPcLoop中了
			tstate <= tstate + 1'b1;
		end

			//暂存为前次时间数据，留作后期使用
		2: begin
			if (tMaxPcLoop > 64'd100_000_000) begin			//如果得到的值大于1S即为无效值
				tMaxPcLoop <= 64'd0;						//原因是定时器开机即启动且中间无停止
			end 
			else if (tMaxPcLoop < tPcLoop) begin
				tMaxPcLoop <= tPcLoop;
			end

			tLastFpgaToPcEnd <= tCurFpgaToPcEnd;
			tstate <= 0;
		end

		default: begin
			tstate <= 0;
		end

		endcase

    end
end



//同步FIFO
sync_fifo_2048x32b u_sync_fifo_2048x32b (
    .clk      (clk),  // input wire clk
    .rst      (~rst_n),  // input wire rst
    .din      (rec_fifo_din  ),  // input wire [31 : 0] din
    .wr_en    (rec_fifo_wren    ),  // input wire wr_en
    .rd_en    (tx_fifo_rden    ),  // input wire rd_en
    .dout     (tx_fifo_dout   ),  // output wire [31 : 0] dout
    .full     (),            // output wire full
    .empty    ()             // output wire empty
    ); 


endmodule
