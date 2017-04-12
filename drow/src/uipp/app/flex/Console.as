// Copyright (c) 2013 Adobe Systems Inc

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

package com.adobe.flascc
{
  import flash.display.DisplayObjectContainer;
  import flash.display.Sprite;
  import flash.display.Stage;
  import flash.display.StageScaleMode;
  import flash.events.Event;

  import com.adobe.flascc.vfs.ISpecialFile;
  import com.adobe.flascc.vfs.LSOBackingStore;

  import flash.display.Stage3D;
  import flash.system.System;

  /**
  * A basic implementation of a console for FlasCC apps.
  * The PlayerKernel class delegates to this for things like read/write
  * so that console output can be displayed in a TextField on the Stage.
  */
  public class Console extends Sprite implements ISpecialFile
  {
    /**
    * To Support the preloader case you might want to have the Console
    * act as a child of some other DisplayObjectContainer.
    */
    public function Console(container:DisplayObjectContainer = null)
    {
        CModule.rootSprite = container ? container.root : this

        if(CModule.runningAsWorker()) {
            return;
        }

        /*flash.system.Security.allowDomain(sameDomain);*/
        
        if(container) {
            container.addChild(this)
            init(null)
        } else {
            addEventListener(Event.ADDED_TO_STAGE, init)
        }
    }
    public function send(value:String):void
    {
        trace(value);
    }

    private function preloadDomainMemory():void {
        var p:int = CModule.malloc(1024*1024*256);
        if (!p) throw(new Error("You have opened too many pages, close some of them or restart your browser!"));
        CModule.malloc(1);//take up the domain memory
        CModule.free(p);//release the pre-allocated memory so that it can be used for new C/C++ Object
    }

    protected function init(e:Event):void {
        //preloadDomainMemory();

        stage.frameRate = 30;
        stage.scaleMode = StageScaleMode.NO_SCALE;

        addEventListener(Event.ENTER_FRAME, this.enterFrame);
        CModule.vfs.console = this;
        CModule.startAsync(this);
    }

    /**
    * The enterFrame callback will be run once every frame. UI thunk requests should be handled
    * here by calling CModule.serviceUIRequests() (see CModule ASdocs for more information on the UI thunking functionality).
    */
    protected function enterFrame(e:Event):void
    {
        CModule.serviceUIRequests()
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C process exit requests
    */
    public function exit(code:int):Boolean {
        try {
            removeEventListener(Event.ENTER_FRAME, this.enterFrame);
            
            var args_empty:Vector.<int> = new Vector.<int>();
            CModule.callI(CModule.getPublicSymbol("do_exit"), args_empty);
            trace("Console: exit complete");
        } catch(e:*) {
            trace("Console: exit exception: " + e);
        }
        return true;
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C IO write requests to the file "/dev/tty" (e.g. output from
    * printf will pass through this function). See the ISpecialFile
    * documentation for more information about the arguments and return value.
    */
    public function write(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int {
      var str:String = CModule.readString(bufPtr, nbyte)
      trace(str)
      return nbyte
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C IO read requests to the file "/dev/tty" (e.g. reads from stdin
    * will expect this function to provide the data). See the ISpecialFile
    * documentation for more information about the arguments and return value.
    */
    public function read(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int {
        if(fd == 0 && nbyte == 1) {
            keybytes.position = kp++
            if(keybytes.bytesAvailable) {
                CModule.write8(bufPtr, keybytes.readUnsignedByte())
                return 1
            } else {
                keybytes.length = 0
                keybytes.position = 0
                kp = 0
            }
        }
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C fcntl requests to the file "/dev/tty" 
    * See the ISpecialFile documentation for more information about the
    * arguments and return value.
    */
    public function fcntl(fd:int, com:int, data:int, errnoPtr:int):int {
        return 0
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C ioctl requests to the file "/dev/tty" 
    * See the ISpecialFile documentation for more information about the
    * arguments and return value.
    */
    public function ioctl(fd:int, com:int, data:int, errnoPtr:int):int {
        return 0;
    }
  }
}
